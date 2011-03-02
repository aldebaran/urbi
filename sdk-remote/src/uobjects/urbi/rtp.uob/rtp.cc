/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// Ortp needs a little help to know it's compiling on WIN32.

// ORTP defines its own version of int32_t and all, so disable ours.
#define LIBPORT_NO_CSTDINT_TYPES

#include <libport/config.h>
#include <libport/detect-win32.h>

#include <libport/lexical-cast.hh>
#include <libport/statistics.hh>
#include <libport/sys/socket.h>
#include <libport/system-warning-push.hh>
#include <ortp/ortp.h>
#include <libport/system-warning-pop.hh>

#include <boost/unordered_map.hpp>

#include <libport/asio.hh>
#include <libport/debug.hh>

#include <serialize/serialize.hh>

#include <urbi/socket.hh>
#include <urbi/uclient.hh>
#include <urbi/uexternal.hh>
#include <urbi/uobject.hh>

#include <urbi/uvalue-serialize.hh>

#include <urbi/customuvar.hh>

using namespace urbi;

GD_CATEGORY(URTP);
extern "C"
{
  // We are using our own network backend, which requires using a bit of the
  // internal ortp API.
  void rtp_session_rtp_parse(RtpSession *session, mblk_t *mp,
                             uint32_t local_str_ts,
                             struct sockaddr *addr, socklen_t addrlen);
  void rtp_session_rtcp_parse(RtpSession *session, mblk_t *mp);
}

class URTPLink;

/** RTP client/server and UObject-RTP bridge.
 * This UObject is capable of sending any UVar changes in a RTP stream, and
 * writing any RTP stream to an UVar.
 * It can also inject messages in remote mode, providing a transparent
 * alternate communication channel between the engine and remote UObjects.
 */
class URTP: public UObject, public UObjectSocket
{
public:
  URTP(const std::string& n);
  ~URTP();
  void init();

  /// @@{
  /// Binary data exchange.
  /// Send any change of given UVar (containing Binary) to this RTP session
  void sendVar(UVar& v);
  /// Send received RTP values of binary type to this UVar.
  void receiveVar(UVar& v);
  /// @@}

  /// Listen to the given UDP port for RTP messages.
  int listen(const std::string& host, const std::string& port);
  /// Connect to the given UDP host/port. This is where messages will be sent.
  void connect(const std::string& host, const std::string& port);
  /// Return RTP session statistics.
  UDictionary stats();
  /// Reset the rtp session.
  void reset();
  /// Send given data to the RTP session.
  void send(const UValue&);
  /// Send urbiscript to be executed by the remote end.
  void sendUrbiscript(const std::string& code);

  /// @@{

    /// Grouped sending of non-binary data over RTP.

  /** Send changes to this non-binary UVar to the remote end.
   *  Will try to group multiple messages in the same packet.
   */
  void groupedSendVar(UVar& v);
  /// Stop monitoring this UVar (previously passed to groupedSendVar()
  void unGroupedSendVar(UVar& v);
  /// Add this (name, value) pair to next group-send message.
  void sendGrouped(const std::string& name, const UValue& v,
                   libport::utime_t timestamp);

  /** Delay in seconds between the time first entry is added to the group, and
   * the time the group is sent.
   */
  CustomUVar<ufloat> commitDelay;

  /// Send the group when UVar with given name is updated (must be in group).
  CustomUVar<std::string> commitTriggerVarName;

  /// @@}

  UEvent onConnectEvent, onErrorEvent;
  /// Force handling incoming data as this type, ignoring reported session type.
  UVar forceType;
  /// Session media type.
  UVar mediaType;
  /// Transmit raw binary if true, prefix with binary keywords if false
  UVar raw;
  /// Session jitter control.
  UVar jitter, jitterAdaptive, jitterTime;
  /// Context of the remote end.
  UVar sourceContext;
  /** Force usage of this header when converting RTP data to UBinary.
   * The UVar set by receiveVar() will *not* be honored until a header or
   * a non-default media type is set.
   */
  UVar forceHeader;
  /// Custom media types. Better hit the dynamic range than unassigned values
  enum MediaTypes {
    RAW_BINARY = 100, //< Binary without keywords
    BINARY     = 101, //< Keywords then binary
    URBISCRIPT = 102, //< urbiscript to execute
    /// List of [name, value] to transmit to local backend. No longuer emited.
    VALUES     = 103,
    SER_VALUES = 104, //< Serialized stream of name value timestamp...
  };
  /// Set a variable name to which binary headers are written to.
  void setHeaderTarget(UVar& v);
  /// Send binary data asynchronously
  UVar async;
  /// Use synchronous socket for sending
  UVar syncSendSocket;
  /// Use raw UDP instead of RTP.
  UVar rawUDP;
  /// Display the list of variable names in group
  std::vector<std::string> groupContent();
  virtual void onError(boost::system::error_code erc);
  virtual void onConnect();
  virtual size_t onRead(const void*, size_t);
  void readFrom(const void*, size_t,  boost::shared_ptr<libport::UDPLink>);
  void onChange(UVar& v);
  void onGroupedChange(UVar& v);
  void onTypeChange(UVar& v);
  void onJitterChange(UVar&);
  void onLogLevelChange(UVar& v);
  void commitGroup();
  void send_(const UValue&);
  void localWrite(const std::string& name, const UValue& val,
                  libport::utime_t timestamp = 0);
  URTPLink* makeSocket();
  // Ctor code, size matters
  void born();
  // Dtor code, size matters
  void die();
  RtpSession* session;
  size_t read_ts;
  size_t write_ts;
  UVar* writeTo;
  UVar* headerTarget;
  // Name of the uvar to deliver to locally.
  UVar localDeliver;
  UVar logLevel;
  libport::Lockable lock; // group protection
  libport::serialize::BinaryOSerializer* groupOArchiver;
  std::ostringstream sOArchiver;
  libport::serialize::BinaryISerializer* groupIArchiver;
  std::istringstream sIArchiver;
  // True if group is empty.
  bool groupEmpty_;
  typedef boost::unordered_map<std::string, UVar*> GroupedVars;
  GroupedVars groupedVars;
  friend class URTPLink;
  libport::Statistics<libport::utime_t> sendTime;
  bool sendMode_; // will this socket be used for synchronous sending.
  std::vector<unsigned short> localPorts;
};

class URTPLink: public libport::Socket
{
public:
  URTPLink(URTP* owner);
  virtual size_t onRead(const void*, size_t);
  URTP* owner_;
};

URTPLink::URTPLink(URTP* owner)
 : libport::Socket(owner->get_io_service())
 , owner_(owner)
{
}

size_t URTPLink::onRead(const void* data, size_t size)
{
  return owner_->onRead(data, size);
}

static
std::string
rtp_id()
{
  return libport::format("URTP_%s_%s", getFilteredHostname(),
          // Under uclibc, each thread has a different pid.
#ifdef __UCLIBC__
   "default"
#else
   getpid()
#endif
   );
}

::urbi::URBIStarter<URTP>
starter_URTP(urbi::isPluginMode() ? "URTP" : rtp_id());

static void bounceSend(UObject*o, const UValue& v)
{
  URTP* r = reinterpret_cast<URTP*>(o);
  r->send(v);
}

static void bounceSendGrouped(UObject*o, const std::string& n, const UValue& v,
                       libport::utime_t ts)
{
  URTP* r = reinterpret_cast<URTP*>(o);
  r->sendGrouped(n, v, ts);
}

URTP::URTP(const std::string& n)
 : UObject(n)
 , UObjectSocket(getCurrentContext()->getIoService())
 , session(0)
 , read_ts(1)
 , write_ts(1)
 , writeTo(0)
 , headerTarget(0)
 , groupOArchiver(0)
 , groupIArchiver(0)
 , groupEmpty_(true)
 , sendMode_(false) // we don't know yet
{
  born();
}

void URTP::born()
{
  // Register us to contextimpl hooks
  ctx_->rtpSendGrouped = &bounceSendGrouped;
  ctx_->rtpSend = &bounceSend;
  GD_FINFO_DUMP("URTP::URTP on %s", this);
  UBindFunction(URTP, init);
  static bool ortpInit = false;
  if (!ortpInit)
  {
    ortp_init();
    ortp_scheduler_init();
    ortp_set_log_level_mask(/*ORTP_DEBUG|ORTP_MESSAGE|*/ORTP_WARNING|ORTP_ERROR);
    ortpInit = true;
    UObject::send("var _rtp_object_name = \"" + __name + "\"|");
  }
  UBindVar(URTP, logLevel);
  UNotifyChange(logLevel, &URTP::onLogLevelChange);
  session=rtp_session_new(RTP_SESSION_SENDRECV);
  rtp_session_set_scheduling_mode(session,0);
  // Should we block for sending?
  rtp_session_set_blocking_mode(session,0);
  rtp_session_set_symmetric_rtp(session,TRUE);
  rtp_session_enable_adaptive_jitter_compensation(session, FALSE);
  rtp_session_set_jitter_compensation(session, 500);
  rtp_session_set_payload_type(session,0);
}

URTP::~URTP()
{
  die();
}

void URTP::die()
{
  GD_FINFO_DUMP("URTP::~URTP on %s", this);
  close();
  libport::BlockLock bl(lock);
  {
    foreach(GroupedVars::value_type& v, groupedVars)
      delete v.second;
    groupedVars.clear();
  }
  foreach(unsigned short lp, localPorts)
    closeUDP(lp);
  delete writeTo;
  delete headerTarget;
  delete groupOArchiver;
  delete groupIArchiver;
}

URTPLink* URTP::makeSocket()
{
  return new URTPLink(this);
}

void URTP::init()
{
  UBindEventRename(URTP, onConnectEvent, "onConnect");
  UBindEventRename(URTP, onErrorEvent, "onError");
  UBindVars(URTP, forceType, mediaType, jitter, jitterAdaptive, jitterTime,
            localDeliver, forceHeader, raw);
  UBindVars(URTP, sourceContext, async, syncSendSocket, rawUDP);
  UBindCacheVar(URTP, commitDelay, ufloat);
  UBindCacheVar(URTP, commitTriggerVarName, std::string);
  async = 0;
  syncSendSocket = 0;
  rawUDP = 0;
  localDeliver = "";
  jitter = 1;
  jitterAdaptive = 1;
  jitterTime = 500;
  raw = 0;
  commitDelay = 0.0001;
  commitTriggerVarName = "";
  forceType = 0;
  forceHeader = "";
  UBindFunctions(libport::Socket, getLocalPort, getRemotePort,
                 getLocalHost, getRemoteHost, isConnected, close);
  UBindFunctions(URTP, sendVar, receiveVar, listen, connect, stats, reset,
                 send, setHeaderTarget, sendUrbiscript);
  UBindFunctions(URTP, groupedSendVar, unGroupedSendVar, sendGrouped,
                 groupContent);
  UNotifyChange(mediaType, &URTP::onTypeChange);
  mediaType = 96;
  // Not cool, but otherwise this line gets executed asynchronously in
  // remote mode which is not early enough.
  rtp_session_set_payload_type(session, 96);
  UNotifyChange(jitter, &URTP::onJitterChange);
  UNotifyChange(jitterAdaptive, &URTP::onJitterChange);
  UNotifyChange(jitterTime, &URTP::onJitterChange);
  jitter = 0;
}

int URTP::listen(const std::string& host, const std::string& port)
{
  GD_SINFO_DUMP("Listening on " << host <<":" << port);
  boost::system::error_code erc;
  unsigned short res =
    Socket::listenUDP(host, port,
                      boost::bind(&URTP::readFrom, this, _1, _2, _3),
                      erc, get_io_service());
  if (erc)
    throw std::runtime_error(erc.message());
  localPorts.push_back(res);
  return res;
}

void URTP::readFrom(const void* data, size_t size,
                    boost::shared_ptr<libport::UDPLink>)
{
  onRead(data, size);
}

void URTP::connect(const std::string& host, const std::string& port)
{
  boost::system::error_code erc = Socket::connect(host, port, true);
  if (erc)
    throw std::runtime_error(erc.message());
#define RTP_SOCKET_CONNECTED (1 << 8)
  session->flags |= RTP_SOCKET_CONNECTED;
}

void URTP::onError(boost::system::error_code erc)
{
  onErrorEvent.emit(erc.message());
}

void URTP::onConnect()
{
  onConnectEvent.emit();
}

void URTP::onChange(UVar& v)
{
  send(v.val());
}

template<typename T>
void
transmitRemoteWrite(const std::string& name, const T& val,
                    libport::utime_t timestamp)
{
  UMessage m(*getDefaultClient());
  m.tag = externalModuleTag;
  m.type = MESSAGE_DATA;
  m.value = new UValue(UList());
  UList& l = *m.value->list;
  l.push_back(UEM_ASSIGNVALUE);
  l.push_back(name);
  l.push_back(val);
  l.push_back(timestamp);
  getDefaultClient()->notifyCallbacks(m);
}

size_t URTP::onRead(const void* data, size_t sz)
{
  GD_SINFO_DUMP(this << " packet of size " << sz);

  unsigned char *payload;
  int payload_size;
  int type;
  mblk_t* res = 0;
  if (rawUDP)
  {
    payload = (unsigned char*)data;
    payload_size = sz;
    type = mediaType;
    if (forceType)
      type = forceType;
  }
  else
  {
    /* Normal operation of ortp is to call rtp_session_recvm_with_ts which will
    * read its socket, get a packet and pass it to rtp_session_rtp_parse.
    * But here we handle the socket ourselve. Fortunately the above sequence
    * runs fine even if the socket is -1.
    */
    mblk_t* mp = allocb(sz, 1);
    memcpy(mp->b_datap->db_base, data, sz);
    mp->b_wptr = mp->b_datap->db_base + sz;
    mp->b_rptr = mp->b_datap->db_base;
    rtp_session_rtp_parse (session, mp, read_ts,
                           (struct sockaddr*)0,
                           0);
    res = rtp_session_recvm_with_ts(session, read_ts);
    if (res)
      read_ts++;
    GD_SINFO_DUMP("rtp_session_recvm_with_ts " << res);
    if (!res)
      return sz; // No data available
    payload_size=rtp_get_payload(mp,&payload);
    type = rtp_get_payload_type(mp);
    // Do not write if the type did not change.
    if (type != (int)mediaType)
      mediaType = type;
    GD_SINFO_DUMP("rtp payload type " << type);
    if (forceType)
      type = forceType;
  }
  std::string ld = localDeliver;

  switch (type)
  {
  case URBISCRIPT:
    UObject::send(std::string((const char*)payload, payload_size));
    break;

  case SER_VALUES:
  {
    sIArchiver.clear();
    sIArchiver.str(std::string((const char*)payload, payload_size));
    GD_FINFO_DUMP("Deserializing SER_VALUES (%s, %s)", (int)(((const char*)payload)[0]), (int)(((const char*)payload)[1]));
    if (!groupIArchiver)
      groupIArchiver = new libport::serialize::BinaryISerializer(sIArchiver);
    try {
    while (!sIArchiver.eof())
    {
      std::string name;
      libport::utime_t timestamp;
      unsigned int tlow, thi;
      UValue val;
      *groupIArchiver >> name >> tlow >> thi >> val;
      timestamp = tlow + ((libport::utime_t)thi << 32);
      if (!name.empty())
        localWrite(name, val, timestamp);
    }
    }
    // EOF detection is buggy. We will land there every turn.
    catch(const libport::serialize::Exception& e)
    {
      GD_FINFO_TRACE("Serialize exception %s", e.what());
    }
  }
  break;
  case VALUES:
  {
    // FIXME: this will not work with binaries
    binaries_type bd;
    UMessage m(*(UClient*)0, 0, "",
               std::string((const char*)payload, payload_size),
               bd);
    if (m.type != MESSAGE_DATA || m.value->type != DATA_LIST)
    {
      GD_WARN("Unexpected RTP message with payload type 'values'");
      return sz;
    }
    foreach(UValue* v, m.value->list->array)
    {
      if (v->type != DATA_LIST)
      {
        GD_WARN("Malformed 'value' RTP message");
        return sz;
      }
      if ((*v->list)[0].type == DATA_STRING)
      {
        if (v->list->size() != 2)
        {
          GD_WARN("Malformed 'value' RTP message");
          return sz;
        }
        std::string name = *v->list->array[0]->stringValue;
        const UValue& val  = *v->list->array[1];
        // Transmit the value to the UObject backend directly
        localWrite(name, val);
      }
      else if ((*v->list)[0].type == DATA_DOUBLE && isRemoteMode())
      {
        UMessage m(*getDefaultClient());
        m.tag = externalModuleTag;
        m.type = MESSAGE_DATA;
        m.value = new UValue(*v->list);
        getDefaultClient()->notifyCallbacks(m);
      }
    }
    break;
  }

  case BINARY:
  case RAW_BINARY:
  case 26:
  {
    if (!writeTo && ld.empty())
      return sz;
    UBinary b;
    binaries_type bd;
    std::string keywords;
    if (type == BINARY)
    {
      // Just honor the included header.
      void* p = memchr(payload, '\n', payload_size);
      if (!p)
      {
        GD_WARN("Parse error in binary message: no newline detected");
        return sz;
      }
      char* start = (char*)p + 1;
      size_t len = start - (char*)payload;
      bd.push_back(BinaryData(start, payload_size - len));
      keywords = string_cast(payload_size-len) + " "
        + std::string((const char*)payload, len+1);
    }
    else
    {
      // Use forceheader, or session type (26=jpeg), otherwise, raw binary.
      std::string fh = forceHeader;
      keywords = string_cast(payload_size);
      if (!fh.empty())
        keywords += " " + fh;
      else if (type == 26)
        keywords += " jpeg";
      keywords += "\n";
      bd.push_back(BinaryData(payload, payload_size));
    }
    binaries_type::const_iterator beg = bd.begin();
    b.parse(keywords.c_str(), 0, bd, beg, false);
    if (writeTo)
    {
      GD_SINFO_DUMP("writing to " << writeTo->get_name());
      std::string sc = sourceContext;
      if (sc.empty())
        *writeTo = b;
      else
        call("uobjects", "$uobject_writeFromContext", sc, writeTo->get_name(),
             b);
    }
    if (!ld.empty())
    {
      // We are in the io_service thread, deliver asynchronously.
      // FIXME: use the client of current context instead.
      GD_SINFO_DUMP("Transmitting " << b.getMessage() <<" " << b.common.size);
      transmitRemoteWrite(ld, b, libport::utime());
    }
    b.common.data = 0;
    if (res)
      freeb(res);
  }
  break;
  }
  return sz;
}

void URTP::localWrite(const std::string& name, const UValue& val,
                      libport::utime_t timestamp)
{
  GD_FINFO_DUMP("localWrite on %s at %s", name, timestamp);
  if (isRemoteMode())
    transmitRemoteWrite(name, val, timestamp);
  else
  {
    std::string sc = sourceContext;
    if (sc.empty())
    {
      UVar var(name);
      var = val;
    }
    else
      call("uobjects", "$uobject_writeFromContext", sc, name, val, timestamp);
  }
}

void URTP::send(const UValue& v)
{
  GD_FINFO_TRACE("URTP::send type %s on %s", v.type, __name);
  // The boost::bind will make a copy of v.
  if (async)
    libport::asyncCall(boost::bind(&URTP::send_, this, v), 0, get_io_service());
  else
    send_(v);
  GD_FINFO_TRACE("URTP::send finished on %s", v.type, __name);
}

void URTP::send_(const UValue& v)
{
  bool sync = syncSendSocket;
  if (!sendMode_ && sync)
  {
    sendMode_ = true;
    rtp_session_set_blocking_mode(session, 1);
  }
  if (sync)
  {
#ifndef WIN32
    int flags = fcntl(getFD(), F_GETFL);
    fcntl(getFD(), F_SETFL, flags & ~O_NONBLOCK);
#endif
  }
  libport::utime_t start = libport::utime();
  bool craw = (int)raw;
  rtp_session_set_payload_type(session, craw? RAW_BINARY:BINARY);
  const UBinary& b = *v.binary;
  // Send new headers if target is defined and if it changed
  if (headerTarget)
  {
    std::string ht = *headerTarget;
    std::string m = b.getMessage();
    if (m != ht)
    {
      *headerTarget = m;
      // If we do not wait, the UDP packet might reach the remote end before
      // the TCP message with the correct header.
      headerTarget->syncValue();
    }
  }
  // We let ortp do the socket sending stuff, so give it the handle.
  session->rtp.socket = getFD();
  int res = 0;
  if (craw)
  {
    if (rawUDP)
    {
      if (sync)
        syncWrite(b.common.data, b.common.size);
      else
        write(b.common.data, b.common.size);
    }
    else
    {
      mblk_t* p = rtp_session_create_packet(session, RTP_FIXED_HEADER_SIZE,
                                            (const unsigned char*)b.common.data,
                                            b.common.size);
      res = rtp_session_sendm_with_ts(session, p, write_ts++);
    }
  }
  else
  {
    std::string s = b.getMessage() + "\n";
    char*d = new char[b.common.size + s.length()];
    memcpy(d, s.c_str(), s.length());
    memcpy(d+s.length(), b.common.data, b.common.size);
    if (rawUDP)
    {
     if (sync)
       syncWrite(d, b.common.size + s.length());
     else
       write(d, b.common.size + s.length());
    }
    else
    {
      mblk_t* p = rtp_session_create_packet(session, RTP_FIXED_HEADER_SIZE,
                                            (const unsigned char*)d,
                                            b.common.size + s.length());
      res = rtp_session_sendm_with_ts(session, p, write_ts++);
    }
    delete[] d;
  }
  GD_SINFO_DUMP("wrote " << res <<" bytes");
  // But reset it so that it will not attempt to call recvfrom() on our socket.
  session->rtp.socket = -1;
  if (sync)
  {
#ifndef WIN32
    int flags = fcntl(getFD(), F_GETFL);
    fcntl(getFD(), F_SETFL, flags | O_NONBLOCK);
#endif
  }
  static bool dstats = getenv("URBI_STATS");
  if (dstats)
  {
    sendTime.add_sample(libport::utime() - start);
    if (sendTime.n_samples() == 100)
    {
      GD_FINFO_DEBUG("rtp send %s: "
                     "mean = %s, max = %s, min = %s, variance = %s",
                     (bool)rawUDP,
                     sendTime.mean(), sendTime.max(),
                     sendTime.min(), sendTime.variance());
      sendTime.resize(0); //reset
    }
  }
}

void URTP::receiveVar(UVar& v)
{
  writeTo = new UVar(v.get_name());
  writeTo->unnotify();
}

void URTP::setHeaderTarget(UVar& v)
{
  headerTarget = new UVar(v.get_name());
  headerTarget->unnotify();
}

void URTP::sendVar(UVar& v)
{
  UNotifyChange(v, &URTP::onChange);
}

UDictionary URTP::stats()
{
  UDictionary res;
  const rtp_stats_t* stats = rtp_session_get_stats(session);
  if (!stats)
    throw std::runtime_error("No statistics returned");
  res["bytesSent"] = stats->sent;
  res["packetSent"] = stats->packet_sent;
  res["dataBytesReceived"] = stats->recv;
  res["bytesReceived"] = stats->hw_recv;
  res["packetReceived"] = stats->packet_recv;
  res["underrun"] = stats->unavaillable;
  res["latePacket"] = stats->outoftime;
  res["packetLoss"] = stats->cum_packet_loss;
  res["invalid"] = stats->bad;
  res["overrun"] = stats->discarded;
  return res;
}

void URTP::onTypeChange(UVar& t)
{
  int type = t;
  rtp_session_set_payload_type(session, type);
}

void URTP::onJitterChange(UVar&)
{
  int j = jitter;
  int ja = jitterAdaptive;
  int jt = jitterTime;
  GD_SINFO_DUMP(this << " Setting jitter = " << j <<"  jittertime = " << jt);
  rtp_session_enable_jitter_buffer(session, j? TRUE:FALSE);
  rtp_session_enable_adaptive_jitter_compensation(session,
                                                  ja?TRUE:FALSE);
  rtp_session_set_jitter_compensation(session, jt);
}

void URTP::reset()
{
  read_ts = 0;
  write_ts = 0;
  rtp_session_reset(session);
}

void URTP::onLogLevelChange(UVar&v)
{
  int level = v;
  int el = 0;
  switch (level)
  {
  default:
  case 4:
    el|= ORTP_DEBUG;
  case 3:
    el |= ORTP_MESSAGE;
  case 2:
    el |= ORTP_WARNING;
  case 1:
    el |= ORTP_ERROR;
  case 0:
    break;
  }
  ortp_set_log_level_mask(el);
}

void URTP::sendUrbiscript(const std::string& s)
{
  rtp_session_set_payload_type(session, URBISCRIPT);
  mblk_t* p = rtp_session_create_packet(session, RTP_FIXED_HEADER_SIZE,
                                        (const unsigned char*)s.c_str(),
                                        s.size());
  // We let ortp do the socket sending stuff, so give it the handle.
  session->rtp.socket = getFD();

  int res = rtp_session_sendm_with_ts(session, p, write_ts++);
  LIBPORT_USE(res);
  GD_SINFO_DUMP("wrote " << res <<" bytes");
  // But reset it so that it will not attempt to call recvfrom() on our socket.
  session->rtp.socket = -1;
}

void URTP::groupedSendVar(UVar& v)
{
  libport::BlockLock bl(lock);
  // This uvar is temporary.
  UVar* nv = new UVar(v.get_name());
  GD_SINFO_DEBUG("testuvar is " << nv << " " << nv->get_temp());
  groupedVars[v.get_name()] = nv;
  UNotifyChange(*nv, &URTP::onGroupedChange);
}

void URTP::unGroupedSendVar(UVar& v)
{
  libport::BlockLock bl(lock);
  GD_SINFO_DEBUG("testuvar unis" << groupedVars[v.get_name()]);
  groupedVars[v.get_name()]->unnotify();
  delete groupedVars[v.get_name()];
  groupedVars.erase(v.get_name());
}

void URTP::onGroupedChange(UVar& v)
{
  // Protect against loopback: If plugin mode, and remote lobby is the same
  // as current lobby, do not send.
  if (isPluginMode())
  {
    // Get lobby we are connected to.
    UVar remoteLobby(__name, "lobbyId"); // lobbyId is set by makeRTPPair().
    // Get current lobby.
    UVar currentLobby("Global", "currentLobbyId");
    // Check if we are in a UVar update.
    UVar inUpdate("Global", "currentRunnerInUObjectUpdate");
    // If in uvar update and same lobby, 'our' remote is the source: do not
    // transmit.
    if ((std::string)remoteLobby == (std::string)currentLobby
        && inUpdate)
      return;
  }
  sendGrouped(v.get_name(), v.val(), v.timestamp());
}

void URTP::sendGrouped(const std::string& name, const UValue& val,
                       libport::utime_t time)
{
  libport::BlockLock bl(lock);
  if (!groupOArchiver)
    groupOArchiver = new libport::serialize::BinaryOSerializer(sOArchiver);
  unsigned int tlow = (unsigned int)time;
  unsigned int thi = (unsigned int)(time >> 32);
  *groupOArchiver
    << name
    << tlow << thi
    << val;
  ufloat cd = commitDelay.data();
  bool empty = groupEmpty_;
  groupEmpty_ = false;
  std::string& cn = commitTriggerVarName.data();
  if (cn == name)
  {
    static bool synchronous_commit = false;
    if (!synchronous_commit)
      std::cerr <<"synchronous commitgroup" << std::endl;
    synchronous_commit = true;
    commitGroup();
  }
  else if (cn.empty() && empty && cd>=0)
    libport::asyncCall(boost::bind(&URTP::commitGroup, this),
                       libport::utime_t(cd * 1000000LL),
                       ctx_->getIoService());
}

void URTP::commitGroup()
{
  libport::utime_t start = libport::utime();
  libport::BlockLock bl(lock);
  std::string s = sOArchiver.str();
  sOArchiver.clear();
  sOArchiver.str("");
  groupEmpty_ = true;
  if (rawUDP)
  {
    write(s.c_str(), s.length());
  }
  else
  {
    rtp_session_set_payload_type(session, SER_VALUES);
    mblk_t* p = rtp_session_create_packet(session, RTP_FIXED_HEADER_SIZE,
                                          (const unsigned char*)s.c_str(),
                                          s.size());
    // We let ortp do the socket sending stuff, so give it the handle.
    session->rtp.socket = getFD();

    int res = rtp_session_sendm_with_ts(session, p, write_ts++);
    LIBPORT_USE(res);
    GD_SINFO_DUMP("wrote " << res <<" bytes");
    // But reset it so that it will not attempt to call recvfrom() on
    // our socket.
    session->rtp.socket = -1;
  }
  static bool dstats = getenv("URBI_STATS");
  if (dstats)
  {
    sendTime.add_sample(libport::utime() - start);
    if (sendTime.n_samples() == 500)
    {
      GD_FINFO_DEBUG("rtp group send: "
                     "mean = %s, max = %s, min = %s, variance = %s",
                     sendTime.mean(), sendTime.max(),
                     sendTime.min(), sendTime.variance());
      sendTime.resize(0); //reset
    }
  }
}


std::vector<std::string>
URTP::groupContent()
{
  std::vector<std::string> res;
  foreach(const GroupedVars::value_type& v, groupedVars)
    res.push_back(v.second->get_name());
  return res;
}
