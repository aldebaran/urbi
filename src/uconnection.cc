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
#include "uqueue.hh"

#include "object/atom.hh" // object::Lobby

UConnection::UConnection (UServer* userver,size_t packetSize)
  : uerror_ (USUCCESS),
    client_ip_ (0),
    closing_ (false),
    receiving_ (false),
    new_data_added_ (false),
    send_queue_ (new UQueue ()),
    recv_queue_ (new UQueue ()),
    packet_size_ (packetSize),
    blocked_ (false),
    // Initial state of the connection: unblocked, not receiving binary.
    active_ (true),
    lobby_ (new object::Lobby (object::State(*this))),
    parser_ (new UParser ()),
    active_command_ (new ast::Nary()),
    server_ (userver)
{
  //FIXME: This would be better done in Lobby ctor, in Urbi maybe.
  lobby_->slot_set(SYMBOL(lobby), lobby_);
  for (int i = 0; i < MAX_ERRORSIGNALS ; ++i)
    error_signals_[i] = false;

  // initialize the connection tag used to reference local variables
  std::ostringstream o;
  o << 'U' << (long) this;
  connection_tag_ = o.str();
}

UConnection::~UConnection ()
{
  DEBUG(("Destroying UConnection..."));
  delete parser_;
  delete send_queue_;
  delete recv_queue_;
  DEBUG(("done\n"));
}

UConnection&
UConnection::initialize()
{
  for (int i = 0; ::HEADER_BEFORE_CUSTOM[i]; ++i)
    send(::HEADER_BEFORE_CUSTOM[i], "start");

  for (int i = 0; ; ++i)
  {
    char buf[1024];
    server_->getCustomHeader(i, buf, sizeof buf);
    if (!buf[0])
      break;
    send(buf, "start");
  }

  for (int i = 0; ::HEADER_AFTER_CUSTOM[i]; ++i)
    send(::HEADER_AFTER_CUSTOM[i], "start");

  {
    char buf[1024];
    snprintf(buf, sizeof buf, "*** ID: %s\n", connection_tag_.c_str());
    send(buf, "ident");

    snprintf(buf, sizeof buf, "%s created", connection_tag_.c_str());
    server_->echo(::DISPLAY_FORMAT, (long)this,
		 "UConnection::initialize", buf);
  }

  server_->loadFile("CLIENT.INI", *recv_queue_);
  new_data_added_ = true;
  return *this;
}

UConnection&
UConnection::send (const char* buf, const char* tag, bool flush)
{
  if (tag)
  {
    std::string pref = make_prefix (tag);
    send_queue (reinterpret_cast<const ubyte*>(pref.c_str()), pref.length());
  }
  if (buf)
  {
    UErrorValue ret = send_queue (reinterpret_cast<const ubyte*>(buf),
				  strlen(buf)).error_get ();

    if (flush && ret != UFAIL)
      this->flush ();

    CONN_ERR_RET(ret);
  }
  return *this;
}

UConnection&
UConnection::send_queue (const ubyte *buffer, int length)
{
  if (closing_)
    CONN_ERR_RET(USUCCESS);

  // Add to Queue
  send_queue_->push(buffer, length);
  CONN_ERR_RET(USUCCESS);
}

UConnection&
UConnection::continue_send ()
{
  if (closing_)
    CONN_ERR_RET(UFAIL);
# if ! defined LIBPORT_URBI_ENV_AIBO
  boost::mutex::scoped_lock lock(mutex_);
# endif
  blocked_ = false;	    // continue_send unblocks the connection.

  int toSend = send_queue_->size(); // nb of bytes to send
  if (toSend > packet_size_)
    toSend = packet_size_;
  if (toSend == 0)
    CONN_ERR_RET(USUCCESS);

  ubyte* popData = send_queue_->front(toSend);

  if (popData != 0)
  {
    int wasSent = effective_send ((const ubyte*)popData, toSend);

    if (wasSent < 0)
      CONN_ERR_RET(UFAIL);
    else if (wasSent == 0 || send_queue_->pop(wasSent) != 0)
      CONN_ERR_RET(USUCCESS);
  }

  CONN_ERR_RET(UFAIL);
}

UConnection&
UConnection::received (const char *s)
{
  return received ((const ubyte*) s, strlen (s));
}

UConnection&
UConnection::received (const ubyte *buffer, int length)
{
  PING();

#if ! defined LIBPORT_URBI_ENV_AIBO
  boost::recursive_mutex::scoped_lock serverLock(server_->mutex);
#endif

  {
    // Lock the connection.
#if ! defined LIBPORT_URBI_ENV_AIBO
    boost::mutex::scoped_lock lock(mutex_);
#endif
    recv_queue_->push(buffer, length);
  }

#if ! defined LIBPORT_URBI_ENV_AIBO
  boost::try_mutex::scoped_try_lock treeLock(tree_mutex_, false);
#endif

#if ! defined LIBPORT_URBI_ENV_AIBO
  if (!treeLock.try_lock())
#endif
  {
    new_data_added_ = true; //server will call us again right after work
    CONN_ERR_RET(USUCCESS);
  }

  UParser& p = parser_get();
  PING();
  if (p.ast_get ())
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
  receiving_ = true;

  // active_command_: The command to be executed (root of the AST).
  // passert (*active_command_, active_command_->empty());

  // If active_command_ is not empty, a runner is still alive, so set
  // obstructed to true. This is a temporary fix to avoid having several
  // runners on the same Nary, until runners become managed outside UConnection.
  bool obstructed = !active_command_->empty();

  // Loop to get all the commands that are ready to be executed.
  for (std::string command = recv_queue_->pop_command();
       !command.empty();
       command = recv_queue_->pop_command())
  {
    server_->setSystemCommand (false);
    int result = p.process (command);
    passert (result, result != -1);
    p.process_errors(active_command_);
    server_->setSystemCommand (true);

    if (ast::Nary* ast = p.ast_take().release())
    {
      ECHO ("parsed: {{{" << *ast << "}}}");
      // Append to the current list.
      active_command_->splice_back(*ast);
      ECHO ("appended: " << *active_command_ << "}}}");
    }
    else
    {
      std::cerr << make_prefix("error")
		<< "the parser returned NULL" << std::endl;
      // FIXME: Is this line usefull ?
      server_->error(::DISPLAY_FORMAT, (long) this,
		    "UConnection::received",
		    "the parser returned NULL\n");
    }
  }

  // FIXME: recv_queue_->clear();

  // Execute the new command.
  if (!obstructed)
    execute ();

  receiving_ = false;
  p.ast_set (0);
#if ! defined LIBPORT_URBI_ENV_AIBO
  treeLock.unlock();
#endif

  CONN_ERR_RET(USUCCESS);
}

UConnection&
UConnection::send (UErrorCode n)
{
  const char* msg = message (UERRORCODE, n);
  UErrorValue result = send(msg, "error").error_get ();
  if (result == USUCCESS)
  {
    char buf[80];
    strncpy (buf, msg, sizeof buf);
    if (strlen (msg) - 1 < sizeof buf)
      //remove the '\n' at the end.
      buf[strlen(msg)-1] = 0;
    server_->error(::DISPLAY_FORMAT, (long)this, "UConnection::error", buf);
  }
  CONN_ERR_RET(result);
}

UConnection&
UConnection::send (UWarningCode n)
{
  const char* msg = message (UWARNINGCODE, n);
  UErrorValue result = send(msg, "warning").error_get ();
  if (result == USUCCESS)
  {
    char buf[80];
    strncpy (buf, msg, sizeof buf);
    if (strlen (msg) - 1 < sizeof buf)
      //remove the '\n' at the end.
      buf[strlen(msg)-1] = 0;
    server_->echoKey("WARNG", ::DISPLAY_FORMAT, (long)this,
		    "UConnection::warning", buf);
  }
  CONN_ERR_RET(result);
}

UConnection&
UConnection::send (object::rObject result, const char* tag, const char* p)
{
  // "Display" the result.
  std::ostringstream os;
  if (p)
    os << p;
  result->print (os);

  if (!os.str ().empty ())
  {
    std::string prefix = make_prefix (tag);
    send (prefix.c_str (), 0, false);
    send (os.str ().c_str (), 0, false);
    endline ();
  }
  return *this;
}

void
UConnection::new_result (object::rObject result)
{
  // The prefix should be (getTag().c_str()) instead of 0.
  // FIXME: the prefix should not be built manually.
  send (result, 0, 0);
}

UConnection&
UConnection::execute ()
{
  PING ();
  if (active_command_->empty())
    return *this;

  ECHO("Command is: {{{" << *active_command_ << "}}}");

  // Our active_command_ is a ast::Nary, we must now "tell" it that
  // it's a top-level Nary so that it can send its results back to the
  // UConnection.  It also entitles the Runner to free this Nary when
  // it has evaluated it.
  active_command_->toplevel_set (true);

  // FIXME: There is an obvious memory leak here
  runner::Runner* runner =
    new runner::Runner(lobby_,
		       lobby_,
		       ::urbiserver->getScheduler (),
		       active_command_);
  runner->start_job ();

  PING ();
  return *this;
}

std::string
UConnection::make_prefix (const char* tag) const
{
  std::ostringstream o;
  char fill = o.fill('0');
  o << '[' << std::setw(8) << server_->lastTime();
  o.fill(fill);
  if (tag && strlen(tag))
    o << ':' << tag;
  o << "] ";

  return o.str ();
}

int
UConnection::effective_send (const ubyte*, int length)
{
  return length;
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

bool
UConnection::send_queue_empty () const
{
  return send_queue_->empty();
}
