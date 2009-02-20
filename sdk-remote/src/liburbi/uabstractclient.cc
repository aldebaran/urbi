/// \file liburbi/uabstractclient.cc

#include <libport/windows.hh>

#include <libport/cstdio>
#include <libport/cstring>
#include <libport/escape.hh>
#include <libport/unistd.h>

#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <cassert>

#include <libport/sys/stat.h>

#include <algorithm>
#include <iostream>
#include <fstream>

#include <boost/lexical_cast.hpp>

#include <libport/lockable.hh>

#include <urbi/uabstractclient.hh>


#define URBI_ERROR_TAG "[error]"
#define URBI_WILDCARD_TAG "[wildcard]"

namespace urbi
{
  std::ostream&
  default_stream()
  {
    return (getDefaultClient()
	    ? ((UAbstractClient*)getDefaultClient())->stream_get()
	    : std::cerr);
  }

  enum UCallbackType
  {
    UCB_,
    UCB_C,
  };

  static UCallbackID nextId;

  class UCallbackWrapperCB: public UCallbackWrapper
  {
    UCallback cb;
  public:
    UCallbackWrapperCB(UCallback cb)
      : cb(cb)
    {
    }
    virtual UCallbackAction operator()(const UMessage& msg)
    {
      return cb(msg);
    }
  };


  class UCallbackWrapperCCB: public UCallbackWrapper
  {
    UCustomCallback cb;
    void * data;
  public:
    UCallbackWrapperCCB(UCustomCallback cb, void* data)
      : cb(cb)
      , data(data)
    {
    }
    virtual UCallbackAction operator()(const UMessage& msg)
    {
      return cb(data, msg);
    }
  };


  class UClientStreambuf: public std::streambuf
  {
  public:
    UClientStreambuf(UAbstractClient* cl)
      : client(cl)
    {
    }

  protected:
    virtual int overflow(int c = EOF);
    // Override std::basic_streambuf<_CharT, _Traits>::xsputn.
    virtual std::streamsize xsputn(const char* s, std::streamsize n);

  private:
    UAbstractClient* client;
  };

  int
  UClientStreambuf::overflow(int c)
  {
    if (c != EOF)
    {
      char ch = static_cast<char>(c);
      xsputn(&ch, 1);
    }
    return c;
  }

  std::streamsize
  UClientStreambuf::xsputn(const char* s, std::streamsize n)
  {
    client->sendBufferLock.lock();
    size_t clen = strlen(client->sendBuffer);
    if (client->buflen < clen + 1 + n)
    {
      //error
      client->sendBufferLock.unlock();
      return 0;
    }
    memcpy(client->sendBuffer + clen, s, n);
    client->sendBuffer[clen+n] = 0;
    if (strpbrk(client->sendBuffer, "&|;,"))
    {
      client->effectiveSend(client->sendBuffer, strlen(client->sendBuffer));
      client->sendBuffer[0] = 0;
    }
    client->sendBufferLock.unlock();
    return n;
  }


  const char* UAbstractClient::CLIENTERROR_TAG="client error";

  /*! Pass the given UMessage to all registered callbacks with the
   * corresponding tag, as if it were comming from the URBI server.
   */
  void
  UAbstractClient::notifyCallbacks(const UMessage &msg)
  {
    listLock.lock();
    bool inc=false;
    for (callbacks_type::iterator it = callbacks_.begin();
	 it!=callbacks_.end(); inc?it:it++, inc=false)
    {
      if (libport::streq(msg.tag.c_str(), it->tag)
	  || (libport::streq(it->tag, URBI_ERROR_TAG)
              && msg.type == MESSAGE_ERROR)
	  || libport::streq(it->tag, URBI_WILDCARD_TAG))
      {
	UCallbackAction ua = it->callback(msg);
	if (ua == URBI_REMOVE)
	{
	  delete &it->callback;
	  it=callbacks_.erase(it);
	  inc = true;
	}
      }
    }
    listLock.unlock();
  }

  /*! Initializes sendBuffer and recvBuffer, and copy host and port.
   \param host    IP address or name of the robot to connect to.
   \param port    TCP port to connect to.
   \param buflen  size of send and receive buffers.
   Implementations should establish the connection in their constructor.
   */
  UAbstractClient::UAbstractClient(const std::string& host,
				   unsigned port,
				   size_t buflen,
				   bool server)
    : std::ostream(new UClientStreambuf(this))
      // Irk, *new...
    , sendBufferLock(*new libport::Lockable())
    , listLock(*new libport::Lockable())
    , host_(host)
    , port_(port)
    , server_(server)
    , buflen(buflen)
    , rc(0)

    , recvBuffer(NULL)
    , recvBufferPosition(0)

    , kernelMajor_(-1)
    , kernelMinor_(-1)
    , binaryBuffer(NULL)
    , parsePosition(0)
    , inString(false)
    , nBracket(0)
    , binaryMode(false)
    , system(false)
    , init_(true)
    , uid_(0)
    , stream_(this)
  {
    stream_get().setf(std::ios::fixed);
    recvBuffer = new char[buflen];
    if (!recvBuffer)
    {
      rc = -1;
      //printf("UAbstractClient::UAbstractClient out of memory\n");
      return;
    }
    recvBuffer[0] = 0;

    sendBuffer = new char[buflen];
    if (!sendBuffer)
    {
      rc = -1;
      //printf("UAbstractClient::UAbstractClient out of memory\n");
      return;
    }
    sendBuffer[0] = 0;
  }

  UAbstractClient::~UAbstractClient()
  {
    // No more default client if delete
    if ((void*)getDefaultClient () == (void*)this)
      setDefaultClient (0);

    delete [] recvBuffer;
    delete [] sendBuffer;
  }

  /*! In threaded environnments, this function lock()s the send buffer so that
   * only the calling thread can call the send functions. Otherwise do
   * nothing.
   */
  int
  UAbstractClient::startPack()
  {
    sendBufferLock.lock();
    return 0;
  }

  int
  UAbstractClient::endPack()
  {
    int res = 0;
    if (size_t len = strlen(sendBuffer))
      res = effectiveSend(sendBuffer, len);
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    return res;
  }


  /*! Multiple commands can be sent in one call.
   */
  int
  UAbstractClient::send(const char* command, ...)
  {
    if (rc)
      return -1;
    va_list arg;
    va_start(arg, command);
    sendBufferLock.lock();
    rc = vpack(command, arg);
    va_end(arg);
    if (rc < 0)
    {
      sendBufferLock.unlock();
      return rc;
    }
    return rc = endPack();
  }

  int
  UAbstractClient::send(UValue& v)
  {
    switch (v.type)
    {
      case DATA_DOUBLE:
	send("%lf", v.val);
	break;
      case DATA_STRING:
	send("\"%s\"", v.stringValue->c_str());
	break;
      case DATA_BINARY:
	if (v.binary->type != BINARY_NONE
	    && v.binary->type != BINARY_UNKNOWN)
	  v.binary->buildMessage();
        sendBinary(v.binary->common.data, v.binary->common.size,
                   v.binary->message);
	break;
      case DATA_LIST:
      {
	send("[");
	size_t sz = v.list->size();
	for (size_t i = 0; i < sz; ++i)
	{
	  send((*v.list)[i]);
	  if (i != sz-1)
	    send(" , ");
	}
	send("]");
      }
      break;
      case DATA_OBJECT:
      {
	send("OBJ %s [", v.object->refName.c_str());
	size_t sz = v.object->size();
	for (size_t i = 0; i < sz; ++i)
	{
	  send("%s :", (*v.object)[i].name.c_str());
	  send(*((*v.object)[i].val) );
	  if (i != sz-1)
	    send(" , ");
	}
	send("]");
      }
      break;
      case DATA_VOID:
	break;
    };
    return 0;
  }

  int
  UAbstractClient::send(std::istream& is)
  {
    if (rc)
      return -1;
    sendBufferLock.lock();
    while (is.good() && !rc)
    {
      is.read(sendBuffer, buflen);
      rc = effectiveSend(sendBuffer, is.gcount());
    }
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    return rc;
  }



  /*! This function must only be called between a startPack()
   and the corresponding endPack(). Data is queued in the send
   buffer, and sent when endPack() is called.
   */
  int
  UAbstractClient::pack(const char* command, ...)
  {
    if (rc)
      return -1;
    va_list arg;
    va_start(arg, command);
    rc = vpack(command, arg);
    va_end(arg);
    return rc;
  }


  int
  UAbstractClient::vpack(const char* command, va_list arg)
  {
    if (rc)
      return -1;
    sendBufferLock.lock();
    if (command)
      vsprintf(&sendBuffer[strlen(sendBuffer)], command, arg);
    else
      sendBuffer[strlen(sendBuffer)] = 0;
    sendBufferLock.unlock();
    return 0;
  }


  int
  UAbstractClient::sendFile(const std::string& f)
  {
    if (f == "/dev/stdin")
      return send(std::cin);
    else
    {
      std::ifstream is(f.c_str(), std::ios::binary);
      if (is.bad())
        return -1;
      else
        return send(is);
    }
  }


  int
  UAbstractClient::sendBin(const void* buffer, size_t len)
  {
    return sendBin(buffer, len, 0);
  }


  int
  UAbstractClient::sendBin(const void* buffer, size_t len,
			   const char* header, ...)
  {
    if (rc)
      return -1;
    sendBufferLock.lock();
    if (header)
    {
      va_list arg;
      va_start(arg, header);
      vpack(header, arg);
      va_end(arg);
      effectiveSend(sendBuffer, strlen(sendBuffer));
    }

    int res = effectiveSend(buffer, len);
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    return res;
  }

  int
  UAbstractClient::sendBinary(const void* data, size_t len,
                              const std::string& header)
  {
    if (kernelMajor() < 2)
      return sendBin(data, len, "BIN %lu %s;",
                     static_cast<unsigned long>(len), header.c_str());
    else
    {
      sendBufferLock.lock();
      (*this) << "Global.Binary.new(\""
              << libport::escape(header)
              << "\", \"\\B(" << len << ")(";
      send(0);
      effectiveSend(data, len);
      (*this) << ")\")";
      send(0);
      sendBufferLock.unlock();
      return rc;
    }
  }

  struct sendSoundData
  {
    char* buffer;
    int bytespersec;
    size_t length;
    size_t pos;
    char* device;
    char* tag;
    char formatString[50];
    USoundFormat format;
    UAbstractClient* uc;
    bool startNotify;
  };

  static UCallbackAction sendSound_(void* cb, const UMessage &msg)
  {
    //the idea is to cut the sound into small chunks,
    //add a header and send each chunk separately
    //create the header.
    // printf("sound message: %s %d\n", msg.systemValue, msg.type);
    static const size_t CHUNK_SIZE = 32 * 8*60;
    // static const int SUBCHUNK_SIZE = CHUNK_SIZE; //1024;


    sendSoundData* s = (sendSoundData*)cb;
    /*
     if (msg.type != MESSAGE_SYSTEM)
     return URBI_CONTINUE;
     if (strstr(msg.systemValue, "start") && s->startNotify==false)
     {
     s->startNotify = true;
     s->uc->notifyCallbacks(UMessage(*s->uc, 0, s->tag, "*** start"));
     }
     if (!strstr(msg.systemValue, "stop"))
     return URBI_CONTINUE;
     */
    /*wavheader wh =
     {
     {'R', 'I', 'F', 'F'},
     44-8,
     {'W', 'A', 'V', 'E'},
     {'f', 'm', 't', ' '},
     16, 1, 1, 16000, 32000, 2, 16,
     {'d', 'a', 't', 'a'},
     0}; // no comment...
     */
    //handle next chunk
    if (s->format == SOUND_WAV && s->pos==0)
      s->pos = sizeof (wavheader);
    size_t tosend = std::min(CHUNK_SIZE, s->length - s->pos);

    //printf("%d start chunk of size %d at offset %d\n", 0, tosend, s->pos);
    int playlength = tosend *1000 / s->bytespersec;
    s->uc->send("%s.val = BIN %lu %s %s;",
		s->device,
		static_cast<unsigned long>
                (tosend + ((s->format == SOUND_WAV)?sizeof (wavheader):0)),
		(s->format == SOUND_WAV)?"wav":"raw",
		s->formatString
      );
    if (s->format == SOUND_WAV)
    {
      wavheader wh;
      memcpy(&wh, s->buffer, sizeof (wh));
      wh.datalength=tosend;
      wh.length=tosend+44-8;
      s->uc->sendBin(&wh, sizeof (wavheader));
    }


#if 0
    // this appears to be useless
    int msecpause = ((SUBCHUNK_SIZE / 32) * 10) / 14;
    int spos = 0;
    while (spos != tosend)
    {
      int ts = SUBCHUNK_SIZE;
      if (ts > tosend-spos)
	ts = tosend-spos;
      printf("%d chunk\n", mtime());
      s->uc->sendBin(s->buffer, ts);
      s->buffer += ts;
      spos += ts;
      usleep(msecpause);
    }
#endif

    s->uc->sendBin(s->buffer+s->pos, tosend);
    s->uc->send("sleep(%s.remain < %d);"
		" %s << ping;", s->device, playlength / 2, msg.tag.c_str());
    // printf("%d end sending chunk\n", 0);
    s->pos += tosend;
    if (s->pos >= s->length)
    {
      //printf("over: %d %d\n", URBI_REMOVE, URBI_CONTINUE);
      //if (s->tag && s->tag[0])
      //  s->uc->notifyCallbacks(UMessage(*s->uc, 0, s->tag, "*** stop"));

      std::string rDevice = (s->device) ? s->device : "speaker";
      std::string message = rDevice + ".val->blend=" +
	rDevice + ".sendsoundsaveblend;";
      s->uc->send(message.c_str ());
      if (s->tag && s->tag[0])
	s->uc->send("%s << 1;", s->tag);
      free(s->buffer);
      free(s->tag);
      free(s->device);
      delete s;
      return URBI_REMOVE;
    }
    return URBI_CONTINUE;
  }

  /*!
   If tag is set, an URBI system "stop" message with this tag will be
   generated when the sound has been played.  The sound data is copied in
   case of asynchronous send, and may be safely deleted as soon as this
   function returns.
   */
  int
  UAbstractClient::sendSound(const char* device, const USound& sound,
			     const char* tag)
  {
    switch (sound.soundFormat)
    {
    case SOUND_MP3:
    case SOUND_OGG:
      // We don't handle chunking for these formats.
      return sendBin(sound.data, sound.size,
		     "%s +report:  %s.val = BIN %lu %s;",
		     tag, device, static_cast<unsigned long>(sound.size),
                     sound.soundFormat == SOUND_MP3 ? "mp3" : "ogg");
      break;

    case SOUND_WAV:
    case SOUND_RAW:
    {
      std::string rDevice = device ? device : "speaker";
      std::string message = "var " + rDevice + ".sendsoundsaveblend = " +
	rDevice + ".val->blend;" + rDevice + ".val->blend=queue;";
      send(message.c_str());
      sendSoundData* s = new sendSoundData();
      char utag[16];
      makeUniqueTag(utag);
      s->bytespersec = sound.channels * sound.rate * (sound.sampleSize / 8);
      s->uc = this;
      s->buffer = static_cast<char*> (malloc (sound.size));
      memcpy(s->buffer, sound.data, sound.size);
      s->length = sound.size;
      s->tag = tag ? strdup(tag) : 0;
      s->device = strdup(device);
      s->pos = 0;
      s->format = sound.soundFormat;
      if (sound.soundFormat == SOUND_RAW)
	sprintf(s->formatString, "%zd %zd %zd %d",
		sound.channels, sound.rate, sound.sampleSize,
		sound.sampleFormat);
      else
	s->formatString[0] = 0;
      s->startNotify = false;
      UCallbackID cid = setCallback(sendSound_, s, utag);
      // Invoke it 2 times to queue sound.
      if (sendSound_(s, UMessage(*this, 0, utag, "*** stop",
				 binaries_type()))
          == URBI_CONTINUE)
      {
	if (sendSound_(s, UMessage(*this, 0, utag, "*** stop",
				   binaries_type()))
            == URBI_REMOVE)
	  deleteCallback(cid);
      }
      else
	deleteCallback(cid);
      return 0;
    }

    default:
      // Unrecognized format.
      return 1;
    }
  }

  UCallbackID
  UAbstractClient::setCallback(UCallback cb, const char* tag)
  {
    return addCallback(tag, *new UCallbackWrapperCB(cb));
  }

  UCallbackID
  UAbstractClient::setCallback(UCustomCallback cb,
			       void* cbData,
			       const char* tag)
  {
    return addCallback(tag, *new UCallbackWrapperCCB(cb, cbData));
  }

  /*! Returns 1 and fills tag on success, 0 on failure
   */
  int
  UAbstractClient::getAssociatedTag(UCallbackID id, char* tag)
  {
    listLock.lock();
    callbacks_type::iterator it =
      std::find(callbacks_.begin(), callbacks_.end(), id);
    if (it == callbacks_.end())
    {
      listLock.unlock();
      return 0;
    }
    strcpy(tag, it->tag);
    listLock.unlock();
    return 1;
  }


  /*! Returns 0 if no callback with this id was found, 1 otherwise.
   */
  int
  UAbstractClient::deleteCallback(UCallbackID id)
  {
    listLock.lock();
    callbacks_type::iterator it =
      std::find(callbacks_.begin(), callbacks_.end(), id);
    if (it == callbacks_.end())
    {
      listLock.unlock();
      return 0;
    }
    delete &(it->callback);
    callbacks_.erase(it);
    listLock.unlock();
    return 1;
  }

  UCallbackID
  UAbstractClient::sendCommand(UCallback cb, const char* cmd, ...)
  {
    char tag[16];
    makeUniqueTag(tag);
    char* mcmd = new char[strlen(cmd) + strlen(tag) + 5];
    sprintf(mcmd, "%s << %s", tag, cmd);
    UCallbackID cid = setCallback(cb, tag);
    sendBufferLock.lock();
    va_list arg;
    va_start(arg, cmd);
    vpack(mcmd, arg);
    va_end(arg);
    int retval = effectiveSend(sendBuffer, strlen(sendBuffer));
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    delete []  mcmd;
    if (retval)
    {
      deleteCallback(cid);
      return UINVALIDCALLBACKID;
    }
    return cid;
  }

  UCallbackID
  UAbstractClient::sendCommand(UCustomCallback cb, void *cbData,
			       const char* cmd, ...)
  {
    char tag[16];
    makeUniqueTag(tag);
    char* mcmd = new char[strlen(cmd) + strlen(tag) + 10];
    sprintf(mcmd, "%s << %s", tag, cmd);
    UCallbackID cid = setCallback(cb, cbData, tag);
    sendBufferLock.lock();
    va_list arg;
    va_start(arg, cmd);
    vpack(mcmd, arg);
    va_end(arg);
    int retval = effectiveSend(sendBuffer, strlen(sendBuffer));
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    delete []mcmd;
    if (retval)
    {
      deleteCallback(cid);
      return UINVALIDCALLBACKID;
    }
    return cid;

  }

  int
  UAbstractClient::putFile(const char* localName, const char* remoteName)
  {
    size_t len;
    struct stat st;
    if (stat(localName, &st) == -1)
      return 1;
    len = st.st_size;
    sendBufferLock.lock();
    if (!remoteName)
      remoteName = localName;
    send("save(\"%s\", \"", remoteName);
    int res = sendFile(localName);
    send("\");");
    sendBufferLock.unlock();
    return res;
  }

  int
  UAbstractClient::putFile(const void* buffer, size_t length,
			   const char* remoteName)
  {
    send("save(\"%s\", \"", remoteName);
    sendBin(buffer, length);
    send("\");");
    sendBufferLock.unlock();
    return 0;
  }

  void
  UAbstractClient::makeUniqueTag(char* tag)
  {
    sprintf(tag, "URBI_%d", ++uid_);
    return;
  }

  /*!
   As long as this function has not returned, neither recvBuffer
   nor recvBufferPos may be modified.
   */
  void
  UAbstractClient::processRecvBuffer()
  {
    while (true)
    {
      if (binaryMode)
      {
	//Receiving binary. Append to binaryBuffer;
	size_t len =
          std::min(recvBufferPosition - endOfHeaderPosition,
                   binaryBufferLength - binaryBufferPosition);
	if (binaryBuffer)
	  memcpy (static_cast<char*> (binaryBuffer) + binaryBufferPosition,
		  recvBuffer + endOfHeaderPosition, len);
	binaryBufferPosition += len;

	if (binaryBufferPosition == binaryBufferLength)
	{
	  //Finished receiving binary.
	  //append
	  BinaryData bd;
	  bd.size = binaryBufferLength;
	  bd.data = binaryBuffer;
	  bins.push_back(bd);
	  binaryBuffer = 0;

	  if (nBracket == 0)
	  {
	    //end of command, send
	    //dumb listLock.lock();
	    UMessage msg(*this, currentTimestamp, currentTag, currentCommand,
			 bins);
	    notifyCallbacks(msg);
	    //unlistLock.lock();

	    while (!bins.empty())
	    {
	      free(bins.front().data);
	      bins.pop_front();
	    }
	    //flush
	    parsePosition = 0;
	    //Move the extra we received
	    memmove(recvBuffer,
		    recvBuffer + endOfHeaderPosition + len,
		    recvBufferPosition - len - endOfHeaderPosition);
	    recvBufferPosition = recvBufferPosition - len - endOfHeaderPosition;
	  }
	  else
	  {
	    // not over yet
	    //leave parseposition where it is
	    //move the extra (parsePosition = endOfHeaderPosition)
	    memmove(recvBuffer+parsePosition,
		    recvBuffer + endOfHeaderPosition + len,
		    recvBufferPosition - len - endOfHeaderPosition);
	    recvBufferPosition = recvBufferPosition - len;
	  }
	  binaryBuffer = 0;
	  binaryMode = false;

	  //Reenter loop.
	  continue;
	}
	else
	{
	  //Not finished receiving binary.
	  recvBufferPosition = endOfHeaderPosition;
	  return;
	}
      }
      else
      {
	// Not in binary mode.
	char* endline =
	  static_cast<char*> (memchr(recvBuffer+parsePosition, '\n',
				     recvBufferPosition - parsePosition));
	if (!endline)
	  return; //no new end of command/start of binary: wait

	if (parsePosition == 0) // parse header
	{
	  // Ignore empty lines.
	  if (endline == recvBuffer)
	  {
	    memmove(recvBuffer, recvBuffer+1, recvBufferPosition - 1);
	    recvBufferPosition--;
	    continue;
	  }
	  int found = sscanf(recvBuffer, "[%d:%64[A-Za-z0-9_.]]",
			     &currentTimestamp, currentTag);
	  if (found != 2)
	  {
	    found = sscanf(recvBuffer, "[%d]", &currentTimestamp);
	    if (found == 1)
	      currentTag[0] = 0;
	    else // failure
	    {
	      std::cerr << "UAbstractClient::read, error parsing header: '"
			<< recvBuffer << '\'' << std::endl;
	      currentTimestamp = 0;
	      strcpy(currentTag, "UNKNWN");
	      //listLock.lock();
	      UMessage msg(*this, 0, URBI_ERROR_TAG,
			   "!!! UAbstractClient::read, fatal error parsing header",
			   binaries_type());
	      notifyCallbacks(msg);
	      //unlistLock.lock();
	    }
	  }

	  currentCommand = strstr(recvBuffer, "]");
	  if (!currentCommand)
          {
	    //reset all
	    nBracket = 0;
	    inString = false;
	    parsePosition = 0;
	    recvBufferPosition = 0;
	    return;
	  }

	  ++currentCommand;
	  while (*currentCommand == ' ')
	    ++currentCommand;
	  system = (*currentCommand == '!' || *currentCommand == '*');
	  parsePosition = (long) currentCommand - (long) recvBuffer;

	  //reinit just to be sure:
	  nBracket = 0;
	  inString = false;
	}

	while (parsePosition < recvBufferPosition)
	{
	  if (inString)
            switch (recvBuffer[parsePosition])
            {
            case '\\':
	      if (parsePosition == recvBufferPosition-1)
		//we cant handle the '\\'
		return;
	      parsePosition+=2; //ignore next character
	      continue;
            case '"':
              inString = false;
              ++parsePosition;
              continue;
            }
	  else
	  {
            switch (recvBuffer[parsePosition])
            {
            case '"':
	      inString = true;
	      ++parsePosition;
	      continue;
	    case '[':
	      ++nBracket;
	      ++parsePosition;
	      continue;
	    case ']':
	      --nBracket;
	      ++parsePosition;
	      continue;
	    case '\n':
              /* XXX: handle '[' in echoed messages or errors nBracket == 0 */
	      if (true)
	      {
		//end of command
		recvBuffer[parsePosition]=0;
		//listLock.lock();
		UMessage msg(*this, currentTimestamp, currentTag,
			     currentCommand,
			     bins);
		notifyCallbacks(msg);
		//unlistLock.lock();
		//prepare for next read, copy the extra
		memmove(recvBuffer, recvBuffer + parsePosition + 1,
			recvBufferPosition - parsePosition - 1);
		// copy beginning of next cmd
		recvBufferPosition = recvBufferPosition - parsePosition - 1;
		recvBuffer[recvBufferPosition] = 0;
		parsePosition = 0;
		while (!bins.empty())
		{
		  free(bins.front().data);
		  bins.pop_front();
		}
		goto line_finished; //restart
	      }
              // this should not happen: \n should have been handled
              // by binary code below
	      std::cerr << "FATAL PARSE ERROR" << std::endl;
            }
	    if (!system && !strncmp(recvBuffer+parsePosition-3, "BIN ", 4))
	    {
	      //very important: scan starts below current point
	      //compute length
	      char* endLength;
	      binaryBufferLength =
                strtol(recvBuffer+parsePosition+1, &endLength, 0);
	      if (endLength == recvBuffer+parsePosition+1)
	      {
		std::cerr << "UClient::read, error parsing bin data length."
			  << std::endl;
		recvBufferPosition = 0;
		return;
	      }
	      //go to end of header
	      while (recvBuffer[parsePosition] !='\n')
		++parsePosition; //we now we will find a \n
	      ++parsePosition;
	      endOfHeaderPosition = parsePosition;
	      binaryMode = true;
	      binaryBuffer = malloc(binaryBufferLength);
	      binaryBufferPosition = 0;
	      break; //restart in binarymode to handle binary
	    }
	  }
	  ++parsePosition;
	}
        line_finished:
	//either we ate all characters, or we were asked to restart
	if (parsePosition == recvBufferPosition)
	  return;
	continue;
      }
    }
  }

  UCallbackID
  UAbstractClient::setWildcardCallback(UCallbackWrapper& callback)
  {
    return addCallback(URBI_WILDCARD_TAG, callback);
  }

  UCallbackID
  UAbstractClient::setErrorCallback(UCallbackWrapper& callback)
  {
    return addCallback(URBI_ERROR_TAG, callback);
  }

  UCallbackID
  UAbstractClient::setClientErrorCallback(UCallbackWrapper& callback)
  {
    return addCallback(CLIENTERROR_TAG, callback);
  }

  UCallbackID UAbstractClient::setCallback(UCallbackWrapper& callback,
					   const char* tag)
  {
    return addCallback(tag, callback);
  }

  UCallbackID UAbstractClient::addCallback(const char* tag,
					   UCallbackWrapper& w)
  {
    listLock.lock();
    UCallbackInfo ci(w);
    strncpy(ci.tag, tag, URBI_MAX_TAG_LENGTH-1);
    ci.tag[URBI_MAX_TAG_LENGTH-1]=0;
    ci.id = ++nextId;
    callbacks_.push_front(ci);
    listLock.unlock();
    return ci.id;
  }

  /*! Generates a client error message and send it to listeners
  \param message an optional string describing the error
  \param erc an optional system error code on which strerror is called
  */
  void
  UAbstractClient::clientError(const char* message, int erc)
  {
    UMessage m(*this);
    m.type = MESSAGE_ERROR;
    std::string msg;
    if (message)
      msg = message;
    if (message && erc)
      msg += " : ";
    if (erc)
      msg += libport::strerror(erc);
    m.message = m.rawMessage = msg; //rawMessage is incorrect but we dont care
    m.timestamp = 0;
    m.tag = CLIENTERROR_TAG;
    notifyCallbacks(m);
  }

  void
  UAbstractClient::onConnection()
  {
    setCallback(*this, &UAbstractClient::setVersion, "__version");
    // We don't know our kernel version yet.
    send("{ var __ver__ = 2; {var __ver__ = 1};"
         "  var __version;"
         "  if (__ver__ == 2) "
         "    __version = Channel.new(\"__version\");"
         "  __version << system.version;"
         "};");
  }

  int
  UAbstractClient::effective_send(const std::string& s)
  {
    return effectiveSend(s.c_str(), s.size());
  }

  int
  UAbstractClient::effective_send(const char* cp)
  {
    return effectiveSend(cp, strlen(cp));
  }

  UCallbackAction
  UAbstractClient::setConnectionID(const UMessage& msg)
  {
    if (msg.type == MESSAGE_DATA && msg.value)
    {
      std::string id = (std::string)*msg.value;
      if (!id.empty())
      {
	connectionID_ = id;
	return URBI_REMOVE;
      }
    }
    return URBI_CONTINUE;
  }

  UCallbackAction
  UAbstractClient::setVersion(const UMessage& msg)
  {
    if (msg.type != MESSAGE_DATA)
      return URBI_CONTINUE;
    assert(msg.value->type == DATA_STRING);
    kernelVersion_ = *msg.value->stringValue;
    size_t sep = kernelVersion_.find_first_of('.');
    try
    {
      kernelMajor_ = boost::lexical_cast<int>(kernelVersion_.substr(0, sep));
      size_t sep2 = kernelVersion_.find_first_of('.', sep+1);
      if (sep2 != kernelVersion_.npos)
        kernelMinor_ =
          boost::lexical_cast<int>(kernelVersion_.substr(sep+1,
                                                         sep2-sep-1));
      else
        kernelMinor_ = 0;
      setCallback(*this, &UAbstractClient::setConnectionID, "__ident");
      if (kernelMajor_ <= 1)
        send("__ident << local.connectionID;");
      else
        send("Channel.new(\"__ident\") << connectionTag.name;");
    }
    catch(boost::bad_lexical_cast&)
    {
      kernelMajor_ = 2;
      kernelMinor_ = 0;
      std::cerr << "failed to parse kernel version string: '"
                << kernelVersion_ << "', assuming "
                << kernelMajor_ << '.' << kernelMinor_ << '.'
                << std::endl;
    }
    return URBI_REMOVE;
  }

  void
  UAbstractClient::waitForKernelVersion () const
  {
    /// FIXME: use a condition.
    while (kernelMajor_ < 0 && !error())
      usleep(100000);
  }

  int
  UAbstractClient::getCurrentTimestamp () const
  {
    return currentTimestamp;
  }

  const std::string&
  UAbstractClient::connectionID () const
  {
    return connectionID_;
  }

  std::string
  getClientConnectionID (const UAbstractClient* cli)
  {
    if (!cli)
      return "";
    return cli->connectionID ();
  }


  /*-----------------.
  | Default client.  |
  `-----------------*/

  UClient* defaultClient = 0;

  UClient* getDefaultClient()
  {
    return defaultClient;
  }

  UClient& get_default_client()
  {
    return *getDefaultClient();
  }

  void setDefaultClient(UClient * cl)
  {
    defaultClient = cl;
  }


  std::ostream&
  unarmorAndSend(const char* a)
  {
    std::ostream& s = default_stream();
    if (strlen(a)>2)
    {
      if (a[0]=='(' && a[strlen(a)-1]==')')
	s.rdbuf()->sputn(a+1, strlen(a)-2);
      else
	s << a; //this is baaad, user forgot the parenthesis but was lucky
    }
    return s;
  }

} // namespace urbi
