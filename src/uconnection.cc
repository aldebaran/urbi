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

#include "libport/ref-pt.hh"

#include "kernel/userver.hh"
#include "kernel/uconnection.hh"
#include "kernel/uvalue.hh"
#include "kernel/uvariable.hh"

#include "parser/uparser.hh"
#include "ubanner.hh"
#include "ubinary.hh"
#include "ubinder.hh"
#include "ucallid.hh"
#include "ucommandqueue.hh"
#include "ucommandqueue.hh"
#include "unamedparameters.hh"
#include "uqueue.hh"
#include "uqueue.hh"

UConnection::UConnection (UServer *userver,
			  int minSendBufferSize,
			  int maxSendBufferSize,
			  int packetSize,
			  int minRecvBufferSize,
			  int maxRecvBufferSize)
  : uerror_(USUCCESS),
    server(userver),
    activeCommand(0),
    // no active command and no last command at start:
    lastCommand(0),
    connectionTag(0),
    functionTag(0),
    clientIP(0),
    killall(false),
    closing(false),
    receiving(false),
    inwork(false),
    newDataAdded(false),
    returnMode(false),
    obstructed(false),
    parser_(new UParser (*this)),
    error_(USUCCESS),
    sendQueue_(new UQueue (minSendBufferSize, maxSendBufferSize,
			   UConnection::ADAPTIVE)),
    recvQueue_(new UCommandQueue (minRecvBufferSize, maxRecvBufferSize,
				  UConnection::ADAPTIVE)),
    packetSize_(packetSize),
    blocked_(false),
    receiveBinary_(false),
    sendAdaptive_(UConnection::ADAPTIVE),
    recvAdaptive_(UConnection::ADAPTIVE),
    // Initial state of the connection: unblocked, not receiving binary.
    active_(true)
{
  for (int i = 0; i < MAX_ERRORSIGNALS ; ++i)
    errorSignals_[i] = false;

  // initialize the connection tag used to reference local variables
  std::ostringstream o;
  o << "U" << (long) this;
  connectionTag = new UString(o.str());
  UVariable* cid =
    new UVariable(o.str().c_str(), "connectionID", o.str().c_str());
  if (cid)
    cid->uservar = false;
}

//! UConnection destructor.
UConnection::~UConnection()
{
  DEBUG(("Destroying UConnection..."));
  if (connectionTag)
  {
    delete server->getVariable(connectionTag->c_str(), "connectionID");
    delete connectionTag;
  }
  delete activeCommand;

  // free bindings
  for (HMvariabletab::iterator i = ::urbiserver->variabletab.begin();
       i != ::urbiserver->variabletab.end(); ++i)
    if (i->second->binder
	&& i->second->binder->removeMonitor(this))
    {
      delete i->second->binder;
      i->second->binder = 0;
    }

  std::list<HMbindertab::iterator> deletelist;
  for (HMbindertab::iterator i = ::urbiserver->functionbindertab.begin();
       i != ::urbiserver->functionbindertab.end();
       ++i)
    if (i->second->removeMonitor(this))
      deletelist.push_back(i);

  for (std::list<HMbindertab::iterator>::iterator i = deletelist.begin();
       i != deletelist.end();
       ++i)
    ::urbiserver->functionbindertab.erase(*i);
  deletelist.clear();

  for (HMbindertab::iterator i = ::urbiserver->eventbindertab.begin();
       i != ::urbiserver->eventbindertab.end();
       ++i)
    if (i->second->removeMonitor(this))
      deletelist.push_back(i);

  for (std::list<HMbindertab::iterator>::iterator i = deletelist.begin();
       i != deletelist.end();
       ++i)
    ::urbiserver->eventbindertab.erase(*i);
  deletelist.clear();

  delete parser_;
  delete sendQueue_;
  delete recvQueue_;
  DEBUG(("done\n"));
}

UConnection&
UConnection::setIP(IPAdd ip)
{
  clientIP = ip;
  return *this;
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
    (*this) << send(::HEADER_BEFORE_CUSTOM[i], "start");

  int i = 0;
  char customHeader[1024];

  do {
    server->getCustomHeader(i, customHeader, 1024);
    if (customHeader[0]!=0)
      (*this) << send(customHeader, "start");
    ++i;
  } while (customHeader[0]!=0);

  for (int i = 0; ::HEADER_AFTER_CUSTOM[i]; ++i)
    (*this) << send(::HEADER_AFTER_CUSTOM[i], "start");
  sprintf(customHeader, "*** ID: %s\n", connectionTag->c_str());
  (*this) << send(customHeader, "ident");

  sprintf(customHeader, "%s created", connectionTag->c_str());
  server->echo(::DISPLAY_FORMAT, (long)this,
	       "UConnection::initialize",
	       customHeader);

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

//! Send a message prefix [time:tag] through the connection
UConnection&
UConnection::operator<< (_Prefix __pref)
{
  return (*this) << sendc (0, 0, (const ubyte*)__pref._tag);
}

UConnection&
UConnection::operator<< (_Send __msg)
{
  if (__msg._tag != 0)
  {
    std::string pref = mkPrefix (__msg._tag);
    __msg._tag = (const ubyte*)pref.c_str ();
    __msg._taglen = pref.length ();
    sendQueue_->mark (); // put a marker to indicate the beginning of a message

    // UErrorValue ret =
    sendc_(__msg._tag, __msg._taglen);
    // .error ();

    //FIXME: check error
  }
  if (__msg._buf != 0)
  {
    UErrorValue ret = sendc_ (__msg._buf, __msg._buflen).error ();
    free ((void*)__msg._buf);

    if (__msg._flush && ret != UFAIL)
      flush ();

    CONN_ERR_RET(ret);
  }
  return (*this);
}

UConnection&
UConnection::operator<< (_ErrorSignal __err)
{
  return (*this).errorSignal_set (__err._n);
}

UConnection&
UConnection::operator<< (_ErrorCheck __err)
{
  return (*this).errorCheckAndSend (__err._n);
}

UConnection&
UConnection::operator<< (_Activate __act)
{
  // Reminder: this code is the same as activate()
  active_ = __act._st;
  return *this;
}

UConnection&
UConnection::operator<< (_SendAdaptative __adap)
{
  return (*this).setSendAdaptive (__adap._val);
}

UConnection&
UConnection::operator<< (_RecvAdaptative __adap)
{
  return (*this).setReceiveAdaptive (__adap._val);
}

UConnection&
UConnection::operator<< (_MsgCode __msg)
{
  // Reminder: this function is a merge of send_warning and send_error
  const char* msg = message (__msg._t, __msg._n);

  UErrorValue result = UFAIL;

  switch (__msg._t)
  {
    case UERRORCODE:
      (*this) << send(msg, "error");
      break;
    case UWARNINGCODE:
      (*this) << send(msg, "warning");
      break;
    default:
      break;
  };

  result = (*this).error ();

  if (result == USUCCESS)
  {
    char buf[80];
    strncpy (buf, msg, sizeof buf);
    if (strlen (msg) - 1 < sizeof buf)
      //remove the '\n' at the end.
      buf[strlen(msg)-1] = 0;

    switch (__msg._t)
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
UConnection::operator<< (UWarningCode __id)
{
  return *this << msg (UWARNINGCODE, __id);
}

UConnection&
UConnection::operator<< (UErrorCode __id)
{
  return *this << msg (UERRORCODE, __id);
}

UConnection&
UConnection::operator<< (_Execute __cmd)
{
  return execute (__cmd._val);
}

UConnection&
UConnection::operator<< (_Append __cmd)
{
  return append (__cmd._val);
}

UConnection&
UConnection::operator<< (_Received __cmd)
{
  return received_ (__cmd._val, __cmd._len);
}

UConnection&
UConnection::operator<< (UConnection& m (UConnection&))
{
  return (*m)(*this);
}

# endif // 1

//! Generate prefix string to be send before a message
std::string
UConnection::mkPrefix (const ubyte* tag) const
{
  std::stringstream ss;
  ss << "["
     << std::setw(8) << std::setfill('0') << (int)server->lastTime()
     << ":"
     << ((tag != 0) ? tag : (const ubyte*)::UNKNOWN_TAG)
     << "] ";

  return ss.str ();
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
    if (result == UMEMORYFAIL)
    {
      errorSignal(UERROR_SEND_BUFFER_FULL);
      server->memoryOverflow = true;
      server->isolate();
    }
    if (result == UFAIL)
      errorSignal(UERROR_SEND_BUFFER_FULL);

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
  boost::mutex::scoped_lock lock(mutex_);
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
    else
      if (wasSent == 0 || sendQueue_->pop(wasSent) != 0)
	CONN_ERR_RET(USUCCESS);
  }

  server->memoryOverflow = true;
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
  return received_((const ubyte*) s, strlen(s));
}

UConnection&
UConnection::received_ (const ubyte *buffer, int length)
{
  PING();
  if (server->memoryOverflow)
  {
    errorSignal(UERROR_MEMORY_OVERFLOW);
    // Block any new incoming command when the system is out of
    // memory
    CONN_ERR_RET(UFAIL);
  }

  bool gotlock = false;
  // If binary append failed to get lock, abort processing.
  bool faillock = false;

  UErrorValue result = UFAIL;

  boost::recursive_mutex::scoped_lock serverLock(server->mutex);
  boost::try_mutex::scoped_try_lock treeLock(treeMutex, false);

  {
    boost::mutex::scoped_lock lock(mutex_);

    if (receiveBinary_)
    {
      // Handle and try to finish the binary transfer.
      int total =
	binCommand->refBinary->ref()->bufferSize - transferedBinary_;
      if (length < total)
      {
	memcpy(binCommand->refBinary->ref()->buffer + transferedBinary_,
	       buffer,
	       length);
	transferedBinary_ += length;
	CONN_ERR_RET(USUCCESS);
      }
      else
      {
	memcpy(binCommand->refBinary->ref()->buffer + transferedBinary_,
	       buffer,
	       total);
	buffer += total;
	length -= total;
	if (treeLock.try_lock())
	{
	  receiveBinary_ = false;
	  append(binCommand->up);
	  gotlock = true;
	}
	else
	  faillock = true;
      }
    }
    result = recvQueue_->push(buffer, length);
  }

  if (result != USUCCESS)
  {
    // Handles memory errors.
    if (result == UFAIL)
    {
      errorSignal(UERROR_RECEIVE_BUFFER_FULL);
      errorSignal(UERROR_RECEIVE_BUFFER_CORRUPTED);
    }

    if (result == UMEMORYFAIL)
    {
      errorSignal(UERROR_RECEIVE_BUFFER_CORRUPTED);
      server->memoryOverflow = true;
      server->isolate();
    }
    CONN_ERR_RET(result);
  }

  if (faillock)
  {
    newDataAdded = true; //server will call us again right after work
    CONN_ERR_RET(USUCCESS);
  }

  if (!gotlock && !treeLock.try_lock())
  {
    newDataAdded = true; //server will call us again right after work
    CONN_ERR_RET(USUCCESS);
  }

  UParser& p = parser();

  //reentrency trouble
  if (p.commandTree)
    CONN_ERR_RET(USUCCESS);

  // Starts processing
  receiving = true;
  server->updateTime();

  do {
    ubyte* command = recvQueue_->popCommand(length);

    if (command == 0 && length==-1)
    {
      recvQueue_->clear();
      length = 0;
    }

    if (length !=0)
    {
      server->systemcommands = false;
      int result = p.process(command, length);
      server->systemcommands = true;

      if (result == -1)
      {
	server->isolate();
	server->memoryOverflow = true;
      }
      server->memoryCheck();

      // Xtrem memory recovery in case of anomaly
      if (server->memoryOverflow && p.commandTree)
      {
	delete p.commandTree;
	p.commandTree = 0;
      }

      // Error Message handling
      if (*p.errorMessage && !server->memoryOverflow)
      {
	// a parsing error occured
	delete p.commandTree;
	p.commandTree = 0;

	(*this) << send(p.errorMessage, "error");

	p.errorMessage[ strlen(p.errorMessage) - 1 ] = 0; // remove '\n'
	p.errorMessage[ 42 ] = 0; // cut at 41 characters
	server->error(::DISPLAY_FORMAT, (long)this,
		      "UConnection::received",
		      p.errorMessage);
      }
      else if (p.commandTree && p.commandTree->command1)
      {
        // Warnings handling
        if (*p.warning && !server->memoryOverflow)
        {
          // a warning was emitted
          (*this) << send(p.warning, "warn ");

          p.errorMessage[ strlen(p.errorMessage) - 1 ] = 0; // remove '\n'
          p.errorMessage[ 42 ] = 0; // cut at 41 characters
          server->error(::DISPLAY_FORMAT, (long)this,
                        "UConnection::received",
                        p.warning);
        }

	// Process "commandTree"

	// ASSIGN_BINARY: intercept and execute immediately
	if (p.binaryCommand)
	{
	  binCommand =
	    dynamic_cast<UCommand_ASSIGN_BINARY*> (p.commandTree->command1);
	  assert (binCommand);

	  ubyte* buffer =
	    recvQueue_->pop(binCommand->refBinary->ref()->bufferSize);

	  if (buffer)
	  {
	    // the binary was all in the queue
	    memcpy(binCommand->refBinary->ref()->buffer,
		   buffer,
		   binCommand->refBinary->ref()->bufferSize);
	  }
	  else
	  {
	    // not all was there, must set receiveBinary mode on
	    transferedBinary_ = recvQueue_->dataSize();
	    memcpy(binCommand->refBinary->ref()->buffer,
		   recvQueue_->pop(transferedBinary_),
		   transferedBinary_);
	    receiveBinary_ = true;
	  }
	}

	// Pile the command
	if (!receiveBinary_)
	{
	  // immediate execution of simple commands
	  if (!obstructed)
	  {
	    p.commandTree->up = 0;
	    p.commandTree->position = 0;
	    PING();
	    execute(p.commandTree);
	    if (p.commandTree &&
		p.commandTree->status == UCommand::URUNNING)
	      obstructed = true;
	  }

	  if (p.commandTree)
	    append(p.commandTree);

	  p.commandTree = 0;
	}
      }
    }
  } while (length != 0
	   && !receiveBinary_
	   && !server->memoryOverflow);

  receiving = false;
  p.commandTree = 0;
  treeLock.unlock();
  if (server->memoryOverflow)
    CONN_ERR_RET(UMEMORYFAIL);

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
  UErrorValue result = ((*this) << send(msg, "error")).error ();
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
  const char*msg = message (UWARNINGCODE, n);
  UErrorValue result = ((*this) << send(msg, "warning")).error ();
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

//! Execute a command
/*! This function executes a regular command.

  \param command is the UCommand to execute.
*/
UCommand*
UConnection::processCommand(UCommand *&command,
			    URunlevel &rl,
			    bool &mustReturn)
{
  mustReturn = false;
  if (command == 0)
    return 0;

  if (rl != UWAITING
      || returnMode
      || command->toDelete)
    return command;

  rl = UEXPLORED;

  // Handle blocked/freezed commands
  if (command->isFrozen())
    return command;

  if (command->isBlocked())
  {
    delete command;
    return 0;
  }

  while (true)
  {
    // timeout, stop , freeze and connection flags initialization
    if (command->startTime == -1)
    {
      command->startTime = server->lastTime();

      for (UNamedParameters *param = command->flags; param; param = param->next)
	if (param->name)
	{
	  if (*param->name == "flagid")
	  {
	    *param->name = "noflag";
	    UValue* tmpID = param->expression->eval(command, this);
	    if (tmpID)
	    {
	      if (tmpID->dataType == DATA_STRING)
		for (std::list<UConnection*>::iterator i =
		       ::urbiserver->connectionList.begin();
		     i != ::urbiserver->connectionList.end();
		     ++i)
		  if ((*i)->isActive()
		      && (*(*i)->connectionTag == *tmpID->str
			  || *tmpID->str == "all"
			  || (*tmpID->str == "other"
			      && !(*(*i)->connectionTag == *connectionTag))))
		    (*i)->append(new UCommand_TREE(UCommand::location(),
						   Flavorable::UAND,
						   command->copy(),
						   0));
	      delete tmpID;
	    }
	    delete command;
	    return 0;
	  }

	  if (*param->name == "flagtimeout")
	  {
	    command->flagType += 1;
	    command->flagExpr1 = param->expression;
	    (*this) << send("!!! Warning: +timeout flag is obsolete."
			     " Use timeout(time) command instead.\n",
			     command->getTag().c_str());
	  }
	  if (*param->name == "flagstop")
	  {
	    command->flagType += 2;
	    command->flagExpr2 = param->expression;
	    (*this) << send("!!! Warning: +stop flag is obsolete."
			     " Use stopif(test) command instead.\n",
			     command->getTag().c_str());
	  }
	  if (*param->name == "flagfreeze")
	  {
	    command->flagType += 4;
	    command->flagExpr4 = param->expression;
	    (*this) << send("!!! Warning: +freeze flag is obsolete."
			     " Use freezeif(test) command instead.\n",
			     command->getTag().c_str());
	  }

	  if (*param->name == "flag"
	      && param->expression
	      && param->expression->val == 10)
	    command->flagType += 8;

	  if (*param->name == "flag"
	      && param->expression
	      && !command->morphed
	      && (param->expression->val == 4 // 4 = +begin
		  || param->expression->val == 1)) // 1 = +report
	    (*this) << send("*** begin\n", command->getTag().c_str());
	}
    }

    bool stopit = false;

    // flag "+timeout"
    if (command->flagType&1)
    {
      UValue *value = command->flagExpr1->eval(command, this);
      if (value &&
	  value->dataType == DATA_NUM &&
	  command->startTime + value->val <= server->lastTime())
	stopit = true;
      delete value;
    }

    // flag "+stop"
    if (command->flagType&2)
    {
      UTestResult testres = booleval(command->flagExpr2->eval(command, this));

      if (testres == UTRUE)
      {
	if (command->flag_nbTrue2 == 0)
	  command->flag_startTrue2 = server->lastTime();
	++command->flag_nbTrue2;
      }
      else
	command->flag_nbTrue2 = 0;

      if ((command->flagExpr2->softtest_time
	   && command->flag_nbTrue2 > 0
	   && (server->lastTime() - command->flag_startTrue2 >=
	       command->flagExpr2->softtest_time->val))
	  || (command->flag_nbTrue2 >0
	      && command->flagExpr2->softtest_time == 0))
	stopit = true;
    }

    // flag "+freeze"
    if (command->flagType&4)
    {
      UTestResult testres = booleval(command->flagExpr4->eval(command, this));

      if (testres == UTRUE)
      {
	if (command->flag_nbTrue4 == 0)
	  command->flag_startTrue4 = server->lastTime();
	++command->flag_nbTrue4;
      }
      else
	command->flag_nbTrue4 = 0;

      if ((command->flagExpr4->softtest_time &&
	   command->flag_nbTrue4 > 0 &&
	   (server->lastTime() - command->flag_startTrue4 >=
	    command->flagExpr4->softtest_time->val)) ||
	  (command->flag_nbTrue4 >0 &&
	   command->flagExpr4->softtest_time==0))
	return command;
    }

    if (stopit)
    {
      if (command == lastCommand)
	lastCommand = command->up;

      delete command;
      return 0;
    }

    // Regular command processing

    if (command->type == UCommand::TREE)
    {
      mustReturn = true;
      return command;
    }
    else
    {
      // != TREE
      UCommand_TREE* morphed_up = command->up;
      UCommand** morphed_position = command->position;

      switch (command->execute(this))
      {
	case UCommand::UCOMPLETED:
	  if (command == lastCommand)
	    lastCommand = command->up;
	  delete command;
	  return 0;

	case UCommand::UMORPH:
	{
	  command->status = UCommand::UONQUEUE;
	  command->morphed = true;

	  UCommand *morphed = command->morph;
	  morphed->myconnection = command->myconnection;
	  morphed->toDelete = command->toDelete;
	  morphed->up = morphed_up;
	  morphed->position = morphed_position;
	  if (command->flags)
	    morphed->flags = command->flags->copy();

	  morphed->setTag(command);

	  if (!command->persistant)
	    delete command;
	  command = morphed;
	  break;
	}

	default:
	  // "+bg" flag
	  // FIXME: Nia?  What the heck is happening here???
	  if ((command->flagType & 8) &&
	      command->status == UCommand::URUNNING)
	    command->status = UCommand::UBACKGROUND;
	  return command;
      }
    }
  }
}

namespace
{
  /// Simplify a UCommand_TREE if possible.
  /// \return whether a simplification was made.
  // FIXME: Should be with UCommand_TREE, not here.
  bool simplify (UCommand_TREE* tree)
  {
    // Do not simplify nodes that hold scoping information
    if (tree->callid)
      return false;

    // left reduction
    if (!tree->command1 && tree->command2)
    {
      ASSERT(tree->position)
	*tree->position = tree->command2;
      tree->command2->up = tree->up;
      tree->command2->position = tree->position;
      tree->command2->background = tree->background;
      tree->command2 = 0;
      return true;
    }

    // right reduction
    // the background hack is here to preserve {at()...} commands.
    if (!tree->command2
	&& tree->command1
	&& tree->command1->status != UCommand::UBACKGROUND)
    {
      ASSERT(tree->position)
	*tree->position = tree->command1;
      tree->command1->up = tree->up;
      tree->command1->position = tree->position;
      tree->command1->background = tree->background;
      tree->command1 = 0;
      return true;
    }
    return false;
  }
}

//! Execute a command tree
/*! This function executes a command tree and
  returns the next node of the tree to process.

  \param tree is the UCommand_TREE to execute.
*/
//FIXME: find, store (and return ?) error if send fails
UConnection&
UConnection::execute(UCommand_TREE*& execCommand)
{
  PING();
  if (!execCommand || closing)
    return *this;

  // There are complications to make this a for loop: occurrences of
  // "continue".
  UCommand_TREE* tree = execCommand;

  while (tree)
  {
    tree->status = UCommand::URUNNING;

    // Requests a +end notification for {...} type of trees
    if (tree->groupOfCommands)
      tree->myconnection = this;

    //check if freezed
    if (tree->isFrozen())
    {
      tree = tree->up;
      continue;
    }

    // BLOCKED/FREEZED COMMANDS
    bool deletecommand = false;
    if (tree->isBlocked())
    {
      tree->runlevel1 = UEXPLORED;
      tree->runlevel2 = UEXPLORED;
      deletecommand = true;
    }

    // COMMAND1

    if (tree->callid && tree->command1 && tree->runlevel1 == UWAITING)
      stack.push_front(tree->callid);

    bool mustReturn;
    tree->command1 = processCommand (tree->command1, tree->runlevel1,
				     mustReturn);
    if (mustReturn)
    {
      tree = dynamic_cast<UCommand_TREE*> (tree->command1);
      assert (tree);
      continue;
    }

    if (tree->callid)
    {
      assert (!stack.empty ());
      stack.pop_front();
      if (returnMode)
      {
	delete tree->command1;
	tree->command1 = 0;
	returnMode = false;
      }
    }

    // COMMAND2
    if (tree->flavor() == Flavorable::UAND
	|| tree->flavor() == Flavorable::UCOMMA
	|| tree->command1 == 0
	|| tree->command1->status == UCommand::UBACKGROUND)
    {
      if (tree == lastCommand)
	obstructed = false;

      bool mustReturn;
      tree->command2 = processCommand (tree->command2,
				       tree->runlevel2,
				       mustReturn);
      if (mustReturn)
      {
	tree = dynamic_cast<UCommand_TREE*> (tree->command2);
	assert (tree);
	continue;
      }
    }

    // STATUS UPDATE

    if (tree->command1 == 0 && tree->command2 == 0
	|| deletecommand)
    {
      if (tree == lastCommand)
	lastCommand = tree->up;
      if (tree == execCommand)
	execCommand = 0;

      if (tree->position)
	*tree->position = 0;

      UCommand_TREE* oldtree = tree;
      tree = tree->up;
      delete oldtree;
      continue;
    }
    else if (((tree->command1 == 0
	       || tree->command1->status == UCommand::UBACKGROUND)
	      && (tree->command2 == 0 ||
		  tree->command2->status == UCommand::UBACKGROUND))
	     || tree->background == true
	     || (tree->flagType&8))
      tree->status = UCommand::UBACKGROUND;

    tree->runlevel1 = UWAITING;
    tree->runlevel2 = UWAITING;

    // REDUCTION

    if (tree != lastCommand && tree != execCommand && !tree->toDelete)
      if (simplify (tree))
      {
	UCommand_TREE* oldtree = tree;
	tree = tree->up; // cannot be zero
	delete oldtree;
	continue;
      }

    // BACK UP
    tree = tree->up;
  }

  if (execCommand &&
      execCommand->command1 == 0 &&
      execCommand->command2 == 0)
  {
    delete execCommand;
    execCommand = 0;
  }
  PING();
  return *this;
}

//! Append a command to the command queue
/*! This function appends a command to the command queue
  activeCommand is the command to process when the system
  wants to process next command. Commands are stored in
  a tree structure, each branch being a ; , & or | command
  seprarator. The lastCommand always point to a ; or ,
  tree where the left side is empty, ready to receive the
  next command to append.
  See UConnection::execute for more details on the way
  commands are stored and processed.

  \param command is the UCommand to append.
*/
//FIXME: error code ?
UConnection&
UConnection::append(UCommand_TREE *command)
{
  if (activeCommand == 0)
  {
    activeCommand = command;
    command->up = 0;
    command->position = 0;
  }
  else
  {
    lastCommand->command2 = command;
    command->up = lastCommand;
    command->position = &(lastCommand->command2);
  }

  lastCommand = command;
  return *this;
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

//! Performs a variable prefix check for local storage in function calls
/*! When a new variable is created inside a connection, it is necessary to
  check if it is not a variable local to some function, in that case it
  must be added to the local stack of this function in order to destroy
  the variable once the function returns.
  This is done by localVariableCheck.
*/
UConnection&
UConnection::localVariableCheck (UVariable *variable)
{
  if (!stack.empty())
  {
    UCallid* cid = stack.front();
    if (variable->getDevicename() == cid->str())
      cid->store(variable);
  }
  return *this;
}
