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

#include "libport/cstring"
#include "libport/cstdio"
#include <cassert>
#include <cstdarg>
#include <sstream>
#include <iomanip>

#include <boost/lexical_cast.hpp>

#include "libport/lockable.hh"
#include "libport/ref-pt.hh"

#include "kernel/userver.hh"
#include "kernel/uconnection.hh"
#include "kernel/uvalue.hh"
#include "kernel/uvariable.hh"

#include "ast/ast.hh"
#include "ast/nary.hh"
#include "object/object.hh"
#include "object/atom.hh"
#include "runner/fwd.hh"
#include "runner/scheduler.hh"
#include "runner/runner.hh"

#include "parser/uparser.hh"
#include "ubanner.hh"
#include "ubinary.hh"
#include "ubinder.hh"
#include "ucallid.hh"
#include "ucommandqueue.hh"
#include "unamedparameters.hh"
#include "uqueue.hh"

#include "object/atom.hh" // object::Context

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
    context_ (new object::Context (object::State(*this)))
{
  for (int i = 0; i < MAX_ERRORSIGNALS ; ++i)
    errorSignals_[i] = false;

  // initialize the connection tag used to reference local variables
  std::ostringstream o;
  o << 'U' << (long) this;
  connectionTag = new UString(o.str());
  UVariable* cid =
    new UVariable(o.str().c_str(), "connectionID", o.str().c_str());
  if (cid)
    cid->uservar = false;
}

//! UConnection destructor.
UConnection::~UConnection ()
{
  DEBUG(("Destroying UConnection..."));
  if (connectionTag)
  {
    delete server->getVariable(connectionTag->c_str(), "connectionID");
    delete connectionTag;
  }

  // free bindings
  for (HMvariabletab::iterator i = ::urbiserver->getVariableTab ().begin();
       i != ::urbiserver->getVariableTab ().end(); ++i)
    if (i->second->binder
	&& i->second->binder->removeMonitor(this))
    {
      delete i->second->binder;
      i->second->binder = 0;
    }

  std::list<HMbindertab::iterator> deletelist;
  for (HMbindertab::iterator i = ::urbiserver->getFunctionBinderTab ().begin();
       i != ::urbiserver->getFunctionBinderTab ().end();
       ++i)
    if (i->second->removeMonitor(this))
      deletelist.push_back(i);

  for (std::list<HMbindertab::iterator>::iterator i = deletelist.begin();
       i != deletelist.end();
       ++i)
    ::urbiserver->getFunctionBinderTab ().erase(*i);
  deletelist.clear();

  for (HMbindertab::iterator i = ::urbiserver->getEventBinderTab ().begin();
       i != ::urbiserver->getEventBinderTab ().end();
       ++i)
    if (i->second->removeMonitor(this))
      deletelist.push_back(i);

  for (std::list<HMbindertab::iterator>::iterator i = deletelist.begin();
       i != deletelist.end();
       ++i)
    ::urbiserver->getEventBinderTab ().erase(*i);
  deletelist.clear();

  delete parser_;
  delete sendQueue_;
  delete recvQueue_;
  DEBUG(("done\n"));
}

void
UConnection::setIP (IPAdd ip)
{
  clientIP = ip;
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

//! UConnection close. Must be redefined by the robot-specific sub class.
/*! The implementation of this function must set 'closing' to true, to
 tell the UConnection to stop sending data.
 */
UErrorValue
UConnection::closeConnection()
{
  closing = true;
  return USUCCESS;
}

//! Initializes the connection, by sending the standard header for URBI
/*! This function must be called once the connection is operational and
 able to send data. It is a requirement for URBI compliance to send
 the header at start, so this function must be called.
 */
void UConnection::initialize()
{
  for (int i = 0; ::HEADER_BEFORE_CUSTOM[i]; ++i)
    send(::HEADER_BEFORE_CUSTOM[i], "start");

  int i = 0;
  char customHeader[1024];

  do {
    server->getCustomHeader(i, customHeader, 1024);
    if (customHeader[0]!=0)
      send(customHeader, "start");
    ++i;
  } while (customHeader[0]!=0);

  for (int i = 0; ::HEADER_AFTER_CUSTOM[i]; ++i)
    send(::HEADER_AFTER_CUSTOM[i], "start");
  sprintf(customHeader, "*** ID: %s\n", connectionTag->c_str());
  send(customHeader, "ident");

  sprintf(customHeader, "%s created", connectionTag->c_str());
  server->echo(::DISPLAY_FORMAT, (long)this,
	       "UConnection::initialize",
	       customHeader);

  server->loadFile("CLIENT.INI", recvQueue_);
  newDataAdded = true;
}

//! Send a message prefix [time:tag] through the connection
UErrorValue
UConnection::sendPrefix (const char* tag)
{
  std::ostringstream o;
  char fill = o.fill('0');
  o << '[' << std::setw(8) << (int) server->lastTime();
  o.fill(fill);
  if (tag)
    o << ':' << tag;
  o << "] ";

  sendQueue_->mark (); // put a marker to indicate the beginning of a message
  sendc(o.str());
  return USUCCESS;
}

//! Send a "\n" through the connection
UErrorValue
UConnection::endline ()
{
  send((const ubyte*)"\n", 1);
  return USUCCESS;
}

UErrorValue
UConnection::send (const char *s, const char* tag)
{
  sendPrefix(tag);
  return send((const ubyte*)s, strlen(s));
}

UErrorValue
UConnection::sendf (const std::string& tag, const char* format, va_list args)
{
  char buf[1024];
  vsnprintf(buf, sizeof buf, format, args);
  return send (buf, tag.c_str());
}

UErrorValue
UConnection::sendf (const std::string& tag, const char* format, ...)
{
  va_list args;
  va_start(args, format);
  return sendf (tag, format, args);
}

UErrorValue
UConnection::send (const ubyte *buffer, int length)
{
  UErrorValue ret = sendc (buffer, length);
  if (ret != UFAIL)
    flush ();
  return ret;
}

/*--------.
| sendc.  |
`--------*/

UErrorValue
UConnection::sendc (const ubyte *buffer, int length)
{
  if (closing)
    return USUCCESS;
  if (sendQueue_->locked ())
    return UFAIL;

  // Add to Queue
  UErrorValue result = sendQueue_->push(buffer, length);
  if (result != USUCCESS)
  {
    if (result == UFAIL)
      errorSignal(UERROR_SEND_BUFFER_FULL);

    sendQueue_->revert ();
    return UFAIL;
  }

  return USUCCESS;
}

UErrorValue
UConnection::sendc (const char *s, const char* tag)
{
  sendPrefix(tag);
  return sendc((const ubyte*)s, strlen(s));
}

/// Flushes the connection buffer into the network
void
UConnection::flush ()
{
  if (!blocked_)
    continueSend();
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
void
UConnection::block ()
{
  blocked_ = true;
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
UErrorValue
UConnection::continueSend ()
{
  libport::BlockLock bl(this); //lock this function
  blocked_ = false;	    // continueSend unblocks the connection.

  int toSend = sendQueue_->dataSize(); // nb of bytes to send
  if (toSend > packetSize_)
    toSend = packetSize_;
  if (toSend == 0)
    return USUCCESS;

  ubyte* popData = sendQueue_->virtualPop(toSend);

  if (popData != 0)
  {
    int wasSent = effectiveSend ((const ubyte*)popData, toSend);

    if (wasSent < 0)
      return UFAIL;
    else if (wasSent == 0 || sendQueue_->pop(wasSent) != 0)
      return USUCCESS;
  }

  server->isolate();

  return UFAIL;
}

UErrorValue
UConnection::received (const char *s)
{
  return received((const ubyte*) s, strlen(s));
}

UErrorValue
UConnection::received (const ubyte *buffer, int length)
{
  PING();

  bool gotlock = false;
  // If binary append failed to get lock, abort processing.
  bool faillock = false;
  libport::BlockLock bl(server);
  // Lock the connection.
  lock();

  UErrorValue result = recvQueue_->push(buffer, length);
  unlock();
  PING();
  if (result != USUCCESS)
  {
    // Handles memory errors.
    if (result == UFAIL)
    {
      errorSignal(UERROR_RECEIVE_BUFFER_FULL);
      errorSignal(UERROR_RECEIVE_BUFFER_CORRUPTED);
    }

    return result;
  }

  if (faillock)
  {
    newDataAdded = true; //server will call us again right after work
    return USUCCESS;
  }

  if (!gotlock && !treeLock.tryLock())
  {
    newDataAdded = true; //server will call us again right after work
    return USUCCESS;
  }

  UParser& p = parser();
  PING();
  if (p.command_tree_get ())
  {
    // This chunk of code seems suspect in k2, meanwhile:
    pabort ("SHOULD NEVER BE HERE");
    PING();
    //reentrency trouble
    treeLock.unlock();
    return USUCCESS;
  }

  // Starts processing
  receiving = true;
  server->updateTime();

  // Code extracted from the UCommandQueue
  std::string command;
  // active_command_: The command to be executed (root of the AST).
  assert (active_command_->empty());

  // Loop to get all the commands that are ready to be executed.
  do {
    command = recvQueue_->popCommand();

    if (command.empty ())
      recvQueue_->clear();
    else
    {
      server->setSystemCommand (false);
      int result = p.process (command);
      assert (result != -1);
      server->setSystemCommand (true);

      // Warnings handling
      if (p.hasWarning())
      {
	send(p.warning_get().c_str(), "warn ");
	server->error(::DISPLAY_FORMAT, (long)this,
		      "UConnection::received",
		      p.warning_get().c_str());
      }

      // Errors handling
      if (p.hasError())
      {
	// FIXME: 2007-07-20: Currently we can't free the commandTree,
	// we might kill function bodies.
	// XXX 2007-07-28: I think that if we get here, it's because there
	// was a parse error so I guess we can safely free the commandTree
	// here, can't we?
	//delete p.commandTree;
	p.command_tree_set (0);

	send(p.error_get().c_str(), "error");
	server->error(::DISPLAY_FORMAT, (long) this,
		      "UConnection::received",
		      p.error_get().c_str());
      }
      else if (!p.command_tree_get ())
      {
	send ("the parser returned NULL\n", "error");
	server->error(::DISPLAY_FORMAT, (long) this,
		      "UConnection::received",
		      "the parser returned NULL\n");
      }
      else
      {
	// Alright so at this point, we have parsed a new command
	// (either a ";" or a ",", in any case it's a Nary) now it's
	// time to append this new command in the AST.
	ast::Nary& parsed_command =
	  dynamic_cast<ast::Nary&> (*p.command_tree_get ());
	ECHO ("parsed: {{{" << parsed_command << "}}}");
	// Append to the current list.
	active_command_->splice_back(parsed_command);
	p.command_tree_set (0);
	ECHO ("appended: " << *active_command_ << "}}}");
      }
    }
  } while (!command.empty ()
	   && !receiveBinary_);

  // Execute the new command.
  if (!obstructed)
    execute ();

  receiving = false;
  p.command_tree_set (0);
  treeLock.unlock();

  return USUCCESS;
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
UErrorValue
UConnection::error (UErrorCode n)
{
  const char* msg = message (n);
  UErrorValue result = send(msg, "error");
  if (result == USUCCESS)
  {
    char buf[80];
    strncpy (buf, msg, sizeof buf);
    if (strlen (msg) - 1 < sizeof buf)
      //remove the '\n' at the end.
      buf[strlen(msg)-1] = 0;
    server->error(::DISPLAY_FORMAT, (long)this, "UConnection::error", buf);
  }
  return result;
}

//! Send a warning message based on the warning number.

/*! This command sends an warning message through the connection, and to
 the server output system, according to the warning number n.

 \param n the warning number. Use the UWarningCode enum. Can be:
 - 0 : Memory overflow warning
 */
UErrorValue
UConnection::warning (UWarningCode n)
{
  const char*msg = message (n);
  UErrorValue result = send(msg, "warning");
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
  return result ;
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
void
UConnection::errorSignal (UErrorCode n)
{
  errorSignals_[(int)n] = true;
}

//! Check if the errorSignal is active and tries to effectively send the message
/*! If the message can be sent, the errorSignal is canceled, otherwise not.
 */
void
UConnection::errorCheck (UErrorCode n)
{
  if (errorSignals_[(int)n]
      && error(n) == USUCCESS)
    errorSignals_[(int)n] = false;
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
void
UConnection::activate()
{
  active_ = true;
}

//! Disactivate the connection
/*! see UConnection::activate() for more details about activation.
 */
void
UConnection::disactivate ()
{
  active_ = false;
}

//! Disactivate the connection
/*! see UConnection::activate() for more details about activation.
 */
bool
UConnection::isActive ()
{
  return active_;
}

void
UConnection::execute ()
{
  using runner::Runner;
  PING ();

  if (active_command_->empty())
    return;

  ECHO("Command is: {{{" << *active_command_ << "}}}");

  Runner* runner = new Runner(context_,
			      ::urbiserver->getScheduler (),
			      active_command_);
  {
    // Our active_command_ is a ast::Nary, we must now "tell" it that it's a
    // top-level Nary so that it can send its results back to the
    // UConnection.
    active_command_->toplevel_set (true);
  }
  ::urbiserver->getScheduler ().schedule_immediately (runner);

  // FIXME: 2007-07-20: Currently we can't free the command,
  // we might kill function bodies.
  ECHO("Clear commands: {{{" << *active_command_ << "}}}");
  //  active_command_->clear();

  PING ();
}

void
UConnection::new_result (object::rObject result)
{
  // "Display" the result.
  std::ostringstream os;
  result->print (os);

  // The prefix should be (getTag().c_str()) instead of 0.
  if (!os.str ().empty ())
  {
    sendc (os.str ().c_str (), 0);
    endline ();
  }
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
void
UConnection::setSendAdaptive (int sendAdaptive)
{
  sendAdaptive_ = sendAdaptive;
  sendQueue_->setAdaptive (sendAdaptive_);
}

//! Sets receiveAdaptive_
void
UConnection::setReceiveAdaptive (int receiveAdaptive)
{
  recvAdaptive_ = receiveAdaptive;
  recvQueue_->setAdaptive (recvAdaptive_);
}

//! Performs a variable prefix check for local storage in function calls
/*! When a new variable is created inside a connection, it is necessary to
 check if it is not a variable local to some function, in that case it
 must be added to the local stack of this function in order to destroy
 the variable once the function returns.
 This is done by localVariableCheck.
 */
void
UConnection::localVariableCheck (UVariable *variable)
{
  if (!stack.empty())
  {
    UCallid* cid = stack.front();
    if (variable->getDevicename() == cid->str())
      cid->store(variable);
  }
}
