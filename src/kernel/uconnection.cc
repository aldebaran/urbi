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
#include <libport/compiler.hh>

#include <libport/config.h>
#include <libport/cstring>
#include <libport/cstdio>
#include <cassert>
#include <cstdarg>
#include <sstream>
#include <iomanip>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/version.hpp>

#include <libport/assert.hh>
#include <libport/foreach.hh>
#include <libport/ref-pt.hh>

#include <ast/nary.hh>
#include <ast/print.hh>

#include <binder/bind.hh>

#include <flower/flower.hh>

#include <kernel/userver.hh>
#include <kernel/uconnection.hh>

#include <object/alien.hh>
#include <object/atom.hh>
#include <object/object.hh>
#include <object/tag-class.hh>
#include <object/urbi-exception.hh>

#include <parser/uparser.hh>
#include <parser/parse-result.hh>

#include <binder/binder.hh>

#include <runner/sneaker.hh>
#include <runner/shell.hh>

#include <kernel/ubanner.hh>
#include <kernel/uqueue.hh>

UConnection::UConnection (UServer& server, size_t packetSize)
  : uerror_ (USUCCESS),
    closing_ (false),
    receiving_ (false),
    new_data_added_ (false),
    server_ (server),
    send_queue_ (new UQueue ()),
    recv_queue_ (new UQueue ()),
    packet_size_ (packetSize),
    blocked_ (false),
    // Initial state of the connection: unblocked, not receiving binary.
    active_ (true),
    lobby_ (new object::Lobby(object::State(*this))),
    parser_ (new parser::UParser ())
{
  //FIXME: This would be better done in Lobby ctor, in Urbi maybe.
  lobby_->slot_set(SYMBOL(lobby), lobby_);
  for (int i = 0; i < MAX_ERRORSIGNALS ; ++i)
    error_signals_[i] = false;

  // initialize the connection tag used to reference local variables
  std::ostringstream o;
  o << 'U' << (long) this;
  connection_tag_ = o.str();
  object::rObject tag = new object::Object();
  tag->proto_add(object::tag_class);
  tag->slot_set(SYMBOL(tag), box(scheduler::rTag,
		new scheduler::Tag(libport::Symbol(connection_tag_))));
  lobby_->slot_set(SYMBOL(connectionTag), tag);

  // Create the shell.
  shell_ = new runner::Shell(lobby_, server_.getScheduler(), SYMBOL(shell));
  shell_->start_job();

  // Create the sneaker if it needs to be.
  dbg::create_sneaker_if_needed(lobby_, server_.getScheduler());
}

UConnection::~UConnection ()
{
  extract_tag(lobby_->slot_get(SYMBOL(connectionTag)))
    ->stop(server_.getScheduler(), object::void_class);
  DEBUG(("Destroying UConnection..."));
  delete parser_;
  delete send_queue_;
  delete recv_queue_;
  shell_->terminate_now();
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
    server_.getCustomHeader(i, buf, sizeof buf);
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
    server_.echo(::DISPLAY_FORMAT, (long)this,
		 "UConnection::initialize", buf);
  }

  server_.load_file("CLIENT.INI", *recv_queue_);
  new_data_added_ = true;
  return *this;
}

UConnection&
UConnection::send (const char* buf, int len, const char* tag, bool flush)
{
  if (tag)
  {
    std::string pref = make_prefix (tag);
    send_queue (pref.c_str(), pref.length());
  }
  if (buf)
  {
    UErrorValue ret = send_queue (buf, len).error_get ();

    if (flush && ret != UFAIL)
      this->flush ();

    CONN_ERR_RET(ret);
  }
  return *this;
}

UConnection&
UConnection::send_queue (const char* buf, size_t len)
{
  if (!closing_)
    send_queue_->push(buf, len);
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

  // nb of bytes to send.
  size_t toSend = std::min(packet_size_, send_queue_->size());
  ECHO(toSend);

  if (!toSend)
    CONN_ERR_RET(USUCCESS);

  if (char* popData = send_queue_->front(toSend))
  {
    ECHO(popData);
    int wasSent = effective_send (popData, toSend);

    if (wasSent < 0)
      CONN_ERR_RET(UFAIL);
    else if (wasSent == 0 || send_queue_->pop(wasSent))
      CONN_ERR_RET(USUCCESS);
  }

  CONN_ERR_RET(UFAIL);
}

UConnection&
UConnection::received (const char* s)
{
  return received (s, strlen (s));
}

UConnection&
UConnection::received (const char* buffer, size_t length)
{
  PING();

#if ! defined LIBPORT_URBI_ENV_AIBO
  boost::recursive_mutex::scoped_lock serverLock(server_.mutex);
#endif

  {
    // Lock the connection.
#if ! defined LIBPORT_URBI_ENV_AIBO
    boost::mutex::scoped_lock lock(mutex_);
#endif
    recv_queue_->push(buffer, length);
  }

#if ! defined LIBPORT_URBI_ENV_AIBO
  // http://www.boost.org/doc/libs/1_35_0/doc/html/thread/changes.html
#if BOOST_VERSION >= 103500
  boost::try_mutex::scoped_try_lock tree_lock(tree_mutex_, boost::defer_lock);
#else
  boost::try_mutex::scoped_try_lock tree_lock(tree_mutex_, false);
#endif // BOOST_VERSION >= 103500
  if (!tree_lock.try_lock())
#endif
  {
    new_data_added_ = true; //server will call us again right after work
    CONN_ERR_RET(USUCCESS);
  }

  parser::UParser& p = parser_get();
  // There should be no tree sitting in the parser.
  //  passert (*p.ast_get(), !p.ast_get());

  // Starts processing
  receiving_ = true;

  ast::rNary active_command = new ast::Nary;
  // Get all the commands that are ready to be executed.
  for (std::string command = recv_queue_->pop_command();
       !command.empty();
       command = recv_queue_->pop_command())
  {
    parser::parse_result_type result(p.parse(command));
    passert(result.get(), result->status != -1);
    result->process_errors(*active_command);

    if (ast::rNary ast = result->ast_get())
    {
      ECHO ("parsed: {{{" << *ast << "}}}");
      ast = binder::bind(flower::flow(ast)).unsafe_cast<ast::Nary>();
      assert(ast);
      ECHO ("bound and flowed: {{{" << *ast << "}}}");
      // Append to the current list.
      active_command->splice_back(ast);
      ECHO ("appended: " << *active_command << "}}}");
    }
    else
      LIBPORT_ECHO("the parser returned NULL:" << std::endl
                   << "{{{" << command << "}}}");
  }

  // Execute the new command.
  execute (active_command);

  receiving_ = false;
  // p.ast_set (0);
#if ! defined LIBPORT_URBI_ENV_AIBO
  tree_lock.unlock();
#endif

  CONN_ERR_RET(USUCCESS);
}

UConnection&
UConnection::send (object::rObject result, const char* tag, const char* p)
{
  // "Display" the result.
  std::ostringstream os;
  if (p)
    os << p;
  runner::Runner& r = server_.getCurrentRunner();
  try
  {
    result = urbi_call(r, result, SYMBOL(asToplevelPrintable));
  }
  catch (object::LookupError&)
  {
    // nothing
  }
  result->print (os, r);

  if (!os.str ().empty ())
  {
    std::string prefix = make_prefix (tag);
    send (prefix.c_str (), (const char*)0, false);
    send_queue (os.str ().c_str(), os.str().length());
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
UConnection::execute (ast::rNary active_command)
{
  PING ();
  if (active_command->empty())
    return *this;

  ECHO("Command is: {{{" << *active_command << "}}}");

  // Our active_command_ is a ast::Nary, we must now "tell" it that
  // it's a top-level Nary so that it can send its results back to the
  // UConnection.  It also entitles the Runner to clear this Nary when
  // it has evaluated it.
  active_command->toplevel_set (true);

  shell_->append_command(const_cast<const ast::Nary*>(active_command.get()));

  PING ();
  return *this;
}

std::string
UConnection::make_prefix (const char* tag) const
{
  std::ostringstream o;
  char fill = o.fill('0');
  o << '[' << std::setw(8) << server_.lastTime() / 1000L;
  o.fill(fill);
  if (tag && strlen(tag))
    o << ':' << tag;
  o << "] ";

  return o.str ();
}

bool
UConnection::has_pending_command () const
{
  return shell_->pending_command_get();
}

void
UConnection::drop_pending_commands ()
{
  shell_->pending_commands_clear();
}

bool
UConnection::send_queue_empty () const
{
  return send_queue_->empty();
}
