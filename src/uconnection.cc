/*! \file uconnection.cc
 *******************************************************************************

 File: uconnection.cc\n
 Implementation of the UConnection class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */
//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include "libport/config.h"
#include "libport/cstring"
#include "libport/cstdio"
#include <cassert>
#include <cstdarg>
#include <sstream>
#include <iomanip>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include "libport/assert.hh"
#include "libport/ref-pt.hh"

#include "kernel/userver.hh"
#include "kernel/uconnection.hh"

#include "ast/ast.hh"
#include "ast/nary.hh"
#include "object/object.hh"
#include "object/atom.hh"
#include "runner/fwd.hh"
#include "runner/runner.hh"

#include "parser/uparser.hh"
#include "ubanner.hh"
#include "ucommandqueue.hh"
#include "uqueue.hh"

#include "object/atom.hh" // object::Lobby

UConnection::UConnection (UServer* userver,
			  int minSendBufferSize,
			  int maxSendBufferSize,
			  int packetSize,
			  int minRecvBufferSize,
			  int maxRecvBufferSize)
  : uerror_ (USUCCESS),
    server (userver),
    active_command_ (new ast::Nary()),
    connectionTag (0),
    functionTag (0),
    clientIP (0),
    killall (false),
    closing (false),
    receiving (false),
    inwork (false),
    newDataAdded (false),
    returnMode (false),
    obstructed (false),
    parser_ (new UParser (*this)),
    sendQueue_ (new UQueue (minSendBufferSize, maxSendBufferSize,
			    UConnection::ADAPTIVE)),
    recvQueue_ (new UCommandQueue (minRecvBufferSize, maxRecvBufferSize,
				   UConnection::ADAPTIVE)),
    packetSize_ (packetSize),
    blocked_ (false),
    receiveBinary_ (false),
    sendAdaptive_ (UConnection::ADAPTIVE),
    recvAdaptive_ (UConnection::ADAPTIVE),
    // Initial state of the connection: unblocked, not receiving binary.
    active_ (true),
    lobby_ (new object::Lobby (object::State(*this)))
{
  //FIXME: This would be better done in Lobby ctor, in Urbi maybe.
  lobby_->slot_set("lobby", lobby_);
  for (int i = 0; i < MAX_ERRORSIGNALS ; ++i)
    errorSignals_[i] = false;

  // initialize the connection tag used to reference local variables
  std::ostringstream o;
  o << 'U' << (long) this;
  connectionTag = new UString(o.str());
}

//! UConnection destructor.
UConnection::~UConnection ()
{
  DEBUG(("Destroying UConnection..."));
  delete connectionTag;
  delete parser_;
  delete sendQueue_;
  delete recvQueue_;
  DEBUG(("done\n"));
}

UConnection&
UConnection::setIP (IPAdd ip)
{
  clientIP = ip;
  return *this;
}

bool
UConnection::has_pending_command () const
{
  return !active_command_->empty();
}

void
UConnection::drop_pending_commands ()
{
  active_command_->clear();
}

//! Initializes the connection, by sending the standard header for URBI
/*! This function must be called once the connection is operational and
  able to send data. It is a requirement for URBI compliance to send
  the header at start, so this function must be called.
*/
UConnection&
UConnection::initialize()
{
  for (int i = 0; ::HEADER_BEFORE_CUSTOM[i]; ++i)
    *this << send(::HEADER_BEFORE_CUSTOM[i], "start");

  for (int i = 0; ; ++i)
  {
    char buf[1024];
    server->getCustomHeader(i, buf, sizeof buf);
    if (!buf[0])
      break;
    *this << send(buf, "start");
  }

  for (int i = 0; ::HEADER_AFTER_CUSTOM[i]; ++i)
    *this << send(::HEADER_AFTER_CUSTOM[i], "start");

  {
    char buf[1024];
    snprintf(buf, sizeof buf, "*** ID: %s\n", connectionTag->c_str());
    *this << send(buf, "ident");

    snprintf(buf, sizeof buf, "%s created", connectionTag->c_str());
    server->echo(::DISPLAY_FORMAT, (long)this,
		 "UConnection::initialize", buf);
  }

  server->loadFile("CLIENT.INI", recvQueue_);
  newDataAdded = true;
  return *this;
}

# if 1 // use connection as stream

UConnection&
UConnection::block (UConnection& c)
{
  return c.block ();
}

//! Send a "\n" through the connection
UConnection&
UConnection::endl (UConnection& c)
{
  return c.endline ();
}

/// Flushes the connection buffer into the network
UConnection&
UConnection::flush (UConnection& c)
{
  return c.flush ();
}

UConnection&
UConnection::continueSend (UConnection& c)
{
  return c.continueSend();
}

UConnection&
UConnection::activate (UConnection& c)
{
  return c << setActivate (true);
}

UConnection&
UConnection::disactivate (UConnection& c)
{
  return c << setActivate (false);
}

UConnection&
UConnection::close (UConnection& c)
{
  return c.closeConnection ();
}

UConnection&
UConnection::operator<< (_Prefix pref)
{
  return *this << sendc (0, 0, (const ubyte*)pref._tag);
}

UConnection&
UConnection::operator<< (_Send msg)
{
  if (msg._tag != 0)
  {
    std::string pref = mkPrefix (msg._tag);
    msg._tag = (const ubyte*)pref.c_str ();
    msg._taglen = pref.length ();
    sendQueue_->mark (); // put a marker to indicate the beginning of a message

    // UErrorValue ret =
    sendc_ (msg._tag, msg._taglen);

    // .error ();

    //FIXME: check error
  }
  if (msg._buf != 0)
  {
    UErrorValue ret = sendc_ (msg._buf, msg._buflen).error ();
    delete [] msg._buf;

    if (msg._flush && ret != UFAIL)
      flush ();

    CONN_ERR_RET(ret);
  }
  return *this;
}

UConnection&
UConnection::operator<< (_ErrorSignal err)
{
  return errorSignal_set (err._n);
}

UConnection&
UConnection::operator<< (_ErrorCheck err)
{
  return errorCheckAndSend (err._n);
}

UConnection&
UConnection::operator<< (_Activate act)
{
  active_ = act._st;
  return *this;
}

UConnection&
UConnection::operator<< (_SendAdaptative adap)
{
  return setSendAdaptive (adap._val);
}

UConnection&
UConnection::operator<< (_RecvAdaptative adap)
{
  return setReceiveAdaptive (adap._val);
}

UConnection&
UConnection::operator<< (_MsgCode mc)
{
  const char* msg = message (mc._t, mc._n);

  UErrorValue result = UFAIL;

  switch (mc._t)
  {
    case UERRORCODE:
      *this << send(msg, "error");
      break;
    case UWARNINGCODE:
      *this << send(msg, "warning");
      break;
    default:
      break;
  };

  result = error ();

  if (result == USUCCESS)
  {
    char buf[80];
    strncpy (buf, msg, sizeof buf);
    if (strlen (msg) - 1 < sizeof buf)
      //remove the '\n' at the end.
      buf[strlen(msg)-1] = 0;

    switch (mc._t)
    {
      case UERRORCODE:
	server->error(::DISPLAY_FORMAT, (long)this, "UConnection::error", buf);
	break;
      case UWARNINGCODE:
	server->echoKey("WARNG", ::DISPLAY_FORMAT, (long)this,
			"UConnection::warning", buf);
	break;
      case UMSGMAX:
	break;
    };
  }
  error_ = result;
  return *this;
}

UConnection&
UConnection::operator<< (UWarningCode id)
{
  return *this << msg (UWARNINGCODE, id);
}

UConnection&
UConnection::operator<< (UErrorCode id)
{
  return *this << msg (UERRORCODE, id);
}

UConnection&
UConnection::operator<< (_Received cmd)
{
  return received_ (cmd._val, cmd._len);
}

UConnection&
UConnection::operator<< (UConnection& m (UConnection&))
{
  return (*m)(*this);
}

# endif // 1

std::string
UConnection::mkPrefix (const ubyte* tag) const
{
  std::ostringstream o;
  char fill = o.fill('0');
  o << '[' << std::setw(8) << (int) server->lastTime();
  o.fill(fill);
  if (tag && strlen(reinterpret_cast<const char*>(tag)))
    o << ':' << tag;
  o << "] ";

  return o.str ();
}

//! Send a buffer through the connection without flushing it.
/*! The function piles the buffer in the sending queue and calls continueSend()
  if the connection is not blocked (blocked means that the connection is not
  ready to send data). The server will try to send the data in the
  sending queue each time the "work" function is called and if the connection
  is not blocked. It is the job of the programmer to let the kernel know when
  the connection is blocked or not, using the "block()" function to block it
  or by calling continueSend() directly to unblock it.

  \param buffer the buffer to send
  \param length the length of the buffer
  \return
  - USUCCESS: successful. The message is in the queue.
  - UFAIL   : could not send the buffer, not enough memory in the
  send queue.
  \sa send(const char*)
*/
UConnection&
UConnection::sendc_ (const ubyte *buffer, int length)
{
  if (closing)
    CONN_ERR_RET(USUCCESS);
  if (sendQueue_->locked ())
    CONN_ERR_RET(UFAIL);

  // Add to Queue
  UErrorValue result = sendQueue_->push(buffer, length);
  if (result != USUCCESS)
  {
    if (result == UFAIL)
    {
      errorSignal_set(UERROR_SEND_BUFFER_FULL);
      server->isolate();
    }

    sendQueue_->revert ();
    CONN_ERR_RET(UFAIL);
  }

  CONN_ERR_RET(USUCCESS);
}

/// Flushes the connection buffer into the network
UConnection&
UConnection::flush ()
{
  if (!blocked_)
    continueSend();
  return *this;
}

//! Returns the state of the connection: blocked or unblocked.
bool
UConnection::isBlocked ()
{
  return blocked_ ;
}

//! Blocks the connection so that send() functions will not call continueSend.
/*! The normal behavior of the send() functions is to pile the data in the
 internal buffer and call continueSend(), except if the connection is
 blocked. */
UConnection&
UConnection::block ()
{
  blocked_ = true;
  return *this;
}


//! Send at most packetSize bytes in the connection, calling effectiveSend()
/*! Must be called when the system tells that the connection is ready to
 accept new data for sending, in order to carry on the processing of the
 sending queue stored in the internal buffer.
 Each call to continueSend sends packetSize bytes (at most) through the real
 connection until the internal buffer is empty.
 \return
 - USUCCESS: successful
 - UFAIL   : effectiveSend() failed or not enough memory
 */
UConnection&
UConnection::continueSend ()
{
  if (closing)
     CONN_ERR_RET(UFAIL);
# if ! defined LIBPORT_URBI_ENV_AIBO
  boost::mutex::scoped_lock lock(mutex_);
# endif
  blocked_ = false;	    // continueSend unblocks the connection.

  int toSend = sendQueue_->dataSize(); // nb of bytes to send
  if (toSend > packetSize_)
    toSend = packetSize_;
  if (toSend == 0)
    CONN_ERR_RET(USUCCESS);

  ubyte* popData = sendQueue_->virtualPop(toSend);

  if (popData != 0)
  {
    int wasSent = effectiveSend ((const ubyte*)popData, toSend);

    if (wasSent < 0)
      CONN_ERR_RET(UFAIL);
    else if (wasSent == 0 || sendQueue_->pop(wasSent) != 0)
      CONN_ERR_RET(USUCCESS);
  }

  server->isolate();

  CONN_ERR_RET(UFAIL);
}

//! Handles an incoming string.
/*! Must be called each time a string is received by the connection.
  \param s the incoming string
  \return UFAIL buffer overflow
  \return UMEMORYFAIL critical memory overflow
  \return USUCCESS otherwise
*/
UConnection&
UConnection::received_ (const char *s)
{
  return received_ ((const ubyte*) s, strlen (s));
}

UConnection&
UConnection::received_ (const ubyte *buffer, int length)
{
  PING();

#if ! defined LIBPORT_URBI_ENV_AIBO
  boost::recursive_mutex::scoped_lock serverLock(server->mutex);
#endif
  UErrorValue result = UFAIL;
  {
    // Lock the connection.
#if ! defined LIBPORT_URBI_ENV_AIBO
    boost::mutex::scoped_lock lock(mutex_);
#endif
    result = recvQueue_->push(buffer, length);
  }

#if ! defined LIBPORT_URBI_ENV_AIBO
  boost::try_mutex::scoped_try_lock treeLock(treeMutex, false);
#endif

  PING();
  if (result != USUCCESS)
  {
    // Handles memory errors.
    if (result == UFAIL)
    {
      errorSignal_set(UERROR_RECEIVE_BUFFER_FULL);
      errorSignal_set(UERROR_RECEIVE_BUFFER_CORRUPTED);
    }
    CONN_ERR_RET(result);
  }

#if ! defined LIBPORT_URBI_ENV_AIBO
  if (!treeLock.try_lock())
#endif
  {
    newDataAdded = true; //server will call us again right after work
    CONN_ERR_RET(USUCCESS);
  }

  UParser& p = parser();
  PING();
  if (p.command_tree_get ())
  {
    // This chunk of code seems suspect in k2, meanwhile:
    pabort ("SHOULD NEVER BE HERE");
    PING();
    //reentrency trouble
#if ! defined LIBPORT_URBI_ENV_AIBO
    treeLock.unlock();
#endif
    CONN_ERR_RET(USUCCESS);
  }

  // Starts processing
  receiving = true;

  // active_command_: The command to be executed (root of the AST).
  // passert (*active_command_, active_command_->empty());

  // Loop to get all the commands that are ready to be executed.
  for (std::string command = recvQueue_->popCommand();
       !command.empty();
       command = recvQueue_->popCommand())
  {
    server->setSystemCommand (false);
    int result = p.process (command);
    assert (result != -1);
    server->setSystemCommand (true);

    // Warnings handling
    while (p.hasWarning())
    {
      active_command_->message_push(p.warning_get(), "warning");
      p.warning_pop();
    }

    // Errors handling
    if (p.hasError())
    {
      delete p.command_tree_get();
      p.command_tree_set (0);

      while (p.hasError())
      {
	active_command_->message_push(p.error_get(), "error");
	p.error_pop();
      }
    }
    else if (!p.command_tree_get ())
    {
      *this << send ("the parser returned NULL\n", "error");
      server->error(::DISPLAY_FORMAT, (long) this,
		    "UConnection::received",
		    "the parser returned NULL\n");
    }
    else
    {
      // We parsed a new command (either a ";" or a ",", in any case
      // it's a Nary).  Append it in the AST.
      ast::Nary& parsed_command =
	dynamic_cast<ast::Nary&> (*p.command_tree_get ());
      ECHO ("parsed: {{{" << parsed_command << "}}}");
      // Append to the current list.
      active_command_->splice_back(parsed_command);
      p.command_tree_set (0);
      ECHO ("appended: " << *active_command_ << "}}}");
    }
  }

  // FIXME: recvQueue_->clear();

  // Execute the new command.
  if (!obstructed)
    execute ();

  receiving = false;
  p.command_tree_set (0);
#if ! defined LIBPORT_URBI_ENV_AIBO
  treeLock.unlock();
#endif

  CONN_ERR_RET(USUCCESS);
}


//! Sends a buffer through the real connection (redefined in the sub class)
/*! Must be defined to implement the effective code that sends a buffer through
 the connection.

 ATTENTION: The buffer received is a short lived buffer. There is no
 warranty whatsoever that it will survive once the function returns. You must
 make a copy of it if your sending method requires to work asynchronously on
 the buffer, after the function has returned.

 \return the number of bytes effectively sent. -1 means that there was an error.
 */
int
UConnection::effectiveSend (const ubyte*, int length)
{
  return length;
}

//! Send an error message based on the error number.
/*! This command sends an error message through the connection, and to the
 server output system, according to the error number n.

 \param n the error number. */
UConnection&
UConnection::send_error (UErrorCode n)
{
  const char* msg = message (UERRORCODE, n);
  UErrorValue result = (*this << send(msg, "error")).error ();
  if (result == USUCCESS)
  {
    char buf[80];
    strncpy (buf, msg, sizeof buf);
    if (strlen (msg) - 1 < sizeof buf)
      //remove the '\n' at the end.
      buf[strlen(msg)-1] = 0;
    server->error(::DISPLAY_FORMAT, (long)this, "UConnection::error", buf);
  }
  CONN_ERR_RET(result);
}

//! Send a warning message based on the warning number.

/*! This command sends an warning message through the connection, and to
 the server output system, according to the warning number n.

 \param n the warning number. Use the UWarningCode enum. Can be:
 - 0 : Memory overflow warning

 \param complement is a complement string added at the end
 of the warning message.
 */
UConnection&
UConnection::send_warning (UWarningCode n)
{
  const char* msg = message (UWARNINGCODE, n);
  UErrorValue result = (*this << send(msg, "warning")).error ();
  if (result == USUCCESS)
  {
    char buf[80];
    strncpy (buf, msg, sizeof buf);
    if (strlen (msg) - 1 < sizeof buf)
      //remove the '\n' at the end.
      buf[strlen(msg)-1] = 0;
    server->echoKey("WARNG", ::DISPLAY_FORMAT, (long)this,
		    "UConnection::warning", buf);
  }
  CONN_ERR_RET(result);
}

//! Set a flag to insure the error will be send.

/*! This command sends an error message through the connection, and
 to the server output system, according to the error number n. The
 difference with the "error" function is that is does not actually
 send the message but set a flag so that the message will be
 sent. The flag is active as long as the message is not actually
 sent. So, using errorSignal is more robust since it guarantees
 that the message will be sent, at all costs.

 \param n the error number. Use the UErrorCode enum.  */
UConnection&
UConnection::errorSignal_set (UErrorCode n)
{
  errorSignals_[(int)n] = true;
  return *this;
}

//! Check if the errorSignal is active and tries to effectively send the message
/*! If the message can be sent, the errorSignal is canceled, otherwise not.
 */
UConnection&
UConnection::errorCheckAndSend (UErrorCode n)
{
  if (errorSignals_[(int)n]
      && (send_error(n).error () == USUCCESS))
    errorSignals_[(int)n] = false;
  return *this;
}

//! Activate the connection
/*! The use of activation is related to OPENR. When a new connection is created
 in OPENR, it is not active immediately. One must wait until a listen call
 returns. For this reason, the connection is set to disactivate until this
 listen call returns. Then, it is activated and the connection can be
 "visible" from the kernel's point of view.

 In normal situations, just ignore this. For example, if your connection is
 usable (send/receive) once it has been created, you can ignore this.
 */
UConnection&
UConnection::activate()
{
  active_ = true;
  return *this;
}

//! Disactivate the connection
/*! see UConnection::activate() for more details about activation.
 */
UConnection&
UConnection::disactivate()
{
  active_ = false;
  return *this;
}

//! Disactivate the connection
/*! see UConnection::activate() for more details about activation.
 */
bool
UConnection::isActive()
{
  return active_;
}

UConnection&
UConnection::execute ()
{
  using runner::Runner;
  PING ();

  if (active_command_->empty())
    return *this;

  ECHO("Command is: {{{" << *active_command_ << "}}}");

  Runner* runner = new Runner(lobby_,
			      lobby_,
			      ::urbiserver->getScheduler (),
			      active_command_);

  // Our active_command_ is a ast::Nary, we must now "tell" it that
  // it's a top-level Nary so that it can send its results back to the
  // UConnection.  It also entitles the Runner to free this Nary when
  // it has evaluated it.
  active_command_->toplevel_set (true);

  ::urbiserver->getScheduler ().add_job (runner);

  PING ();
  return *this;
}

void
UConnection::send (object::rObject result, const char* tag, const char* p)
{
  // "Display" the result.
  std::ostringstream os;
  if (p)
    os << p;
  result->print (os);

  if (!os.str ().empty ())
  {
    *this
      << sendc (mkPrefix (reinterpret_cast<const ubyte*>(tag)).c_str (), 0)
      << sendc (os.str ().c_str (), 0)
      << endl;
  }
}

void
UConnection::new_result (object::rObject result)
{
  // The prefix should be (getTag().c_str()) instead of 0.
  // FIXME: the prefix should not be built manually.
  send (result, 0, 0);
}

//! Returns how much space is available in the send queue
int
UConnection::availableSendQueue ()
{
  return sendQueue_->bufferMaxFreeSpace();
}


//! Returns how many bytes are still in the send queue
int
UConnection::sendQueueRemain ()
{
  return sendQueue_->dataSize();
}

//! Sets sendAdaptive_
UConnection&
UConnection::setSendAdaptive (int sendAdaptive)
{
  sendAdaptive_ = sendAdaptive;
  sendQueue_->setAdaptive (sendAdaptive_);
  return *this;
}

//! Sets receiveAdaptive_
UConnection&
UConnection::setReceiveAdaptive (int receiveAdaptive)
{
  recvAdaptive_ = receiveAdaptive;
  recvQueue_->setAdaptive (recvAdaptive_);
  return *this;
}
