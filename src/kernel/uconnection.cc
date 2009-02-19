/// \file kernel/uconnection.cc
/// \brief Implementation of UConnection.
//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include <libport/config.h>
#include <libport/cstring>
#include <libport/cstdio>
#include <cassert>
#include <sstream>
#include <iomanip>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/version.hpp>

#include <libport/assert.hh>
#include <libport/foreach.hh>
#include <libport/ref-pt.hh>

#include <ast/nary.hh>
#include <ast/print.hh>

#include <kernel/userver.hh>
#include <kernel/uconnection.hh>

#include <object/lobby.hh>
#include <object/object.hh>
#include <object/state.hh>
#include <object/tag.hh>
#include <object/urbi-exception.hh>

#include <parser/uparser.hh>
#include <parser/parse-result.hh>
#include <parser/transform.hh>

#include <binder/binder.hh>

#include <runner/call.hh>
#include <runner/sneaker.hh>
#include <runner/shell.hh>

#include <kernel/uqueue.hh>

namespace kernel
{
  UConnection::UConnection(UServer& server, size_t packetSize)
    : uerror_(USUCCESS)
    , closing_(false)
    , receiving_(false)
    , new_data_added_(false)
    , server_(server)
    , send_queue_(new UQueue())
    , recv_queue_(new UQueue())
    , packet_size_(packetSize)
    , blocked_(false)
      // Initial state of the connection: unblocked, not receiving binary.
    , active_(true)
    , lobby_(new object::Lobby(object::State(*this)))
    , parser_(new parser::UParser())
  {
    //FIXME: This would be better done in Lobby ctor, in Urbi maybe.
    lobby_->slot_set(SYMBOL(lobby), lobby_);

    // initialize the connection tag used to reference local variables
    std::ostringstream o;
    o << 'U' << (ptrdiff_t) this;
    connection_tag_ = o.str();
    lobby_->slot_set
      (SYMBOL(connectionTag),
       new object::Tag(new sched::Tag(libport::Symbol(connection_tag_))));

    // Create the shell.
    shell_ = new runner::Shell(lobby_, server_.scheduler_get(), SYMBOL(shell));
    shell_->start_job();

    // Create the sneaker if it needs to be.
    dbg::create_sneaker_if_needed(lobby_, server_.scheduler_get());
  }

  UConnection::~UConnection()
  {
    lobby_->slot_remove(SYMBOL(lobby));
    lobby_->slot_get(SYMBOL(connectionTag))->as<object::Tag>()->value_get()
      ->stop(server_.scheduler_get(), object::void_class);
    shell_->terminate_now();
  }

  void
  UConnection::initialize()
  {
    const std::string& banner = server_.banner_get();
    std::vector<std::string> lines;
    boost::split(lines, banner, boost::is_any_of("\n"));
    foreach (const std::string& l, lines)
      send(("*** " + l + "\n").c_str(), "start");

    send(("*** ID: " + connection_tag_ + "\n").c_str(), "ident");

    server_.load_file("CLIENT.INI", *recv_queue_);
    new_data_added_ = true;
  }

  void
  UConnection::send(const char* buf, size_t len, const char* tag, bool flush_p)
  {
    if (tag)
    {
      std::string pref = make_prefix(tag);
      send_queue(pref.c_str(), pref.length());
    }
    if (buf)
    {
      send_queue(buf, len);
      UErrorValue res = error_;

      if (flush_p && res != UFAIL)
        flush();

      error_ = res;
    }
  }

  void
  UConnection::send_queue(const char* buf, size_t len)
  {
    if (!closing_)
      send_queue_->push(buf, len);
    error_ = USUCCESS;
  }

  void
  UConnection::continue_send()
  {
    if (closing_)
    {
      error_ = UFAIL;
      return;
    }
    blocked_ = false;	    // continue_send unblocks the connection.

    // nb of bytes to send.
    size_t toSend = std::min(packet_size_, send_queue_->size());
    ECHO(toSend);

    if (!toSend)
    {
      error_ = USUCCESS;
      return;
    }

    if (const char* popData = send_queue_->peek(toSend))
    {
      ECHO(popData);
      int wasSent = effective_send(popData, toSend);

      // FIXME: This can never happen, as effective_send
      // returns a size_t which cannot be negative.
      if (wasSent < 0)
      {
        error_ = UFAIL;
        return;
      }
      else if (wasSent == 0 || send_queue_->pop(wasSent))
      {
        error_ = USUCCESS;
        return;
      }
    }

    error_ = UFAIL;
  }

  void
  UConnection::received(const char* buffer, size_t length)
  {
    PING();

    recv_queue_->push(buffer, length);
    parser::UParser& p = parser_get();

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
        ECHO("parsed: {{{" << *ast << "}}}");
        ast = parser::transform(ast);
        assert(ast);
        ECHO("bound and flowed: {{{" << *ast << "}}}");
        // Append to the current list.
        active_command->splice_back(ast);
        ECHO("appended: " << *active_command << "}}}");
      }
      else
        ECHO("the parser returned NULL:" << std::endl
             << "{{{" << command << "}}}");
    }

    // Execute the new command.
    execute(active_command);

    receiving_ = false;

    error_ = USUCCESS;
  }

  void
  UConnection::send(object::rObject result, const char* tag, const char* p)
  {
    // "Display" the result.
    std::ostringstream os;
    if (p)
      os << p;
    try
    {
      result = urbi_call(result, SYMBOL(asToplevelPrintable));
    }
    catch (object::UrbiException&)
    {
      // nothing
    }
    result->print(os);

    if (!os.str().empty())
    {
      std::string prefix = make_prefix(tag);
      send(prefix.c_str(), (const char*)0, false);
      send_queue(os.str().c_str(), os.str().length());
      endline();
    }
  }

  void
  UConnection::new_result(object::rObject result)
  {
    // The prefix should be(getTag().c_str()) instead of 0.
    // FIXME: the prefix should not be built manually.
    send(result, 0, 0);
  }


  void
  UConnection::execute(ast::rNary active_command)
  {
    PING();
    if (active_command->empty())
      return;

    ECHO("Command is: {{{" << *active_command << "}}}");

    // Our active_command_ is a ast::Nary, we must now "tell" it that
    // it's a top-level Nary so that it can send its results back to the
    // UConnection.  It also entitles the Runner to clear this Nary when
    // it has evaluated it.
    active_command->toplevel_set(true);

    shell_->append_command(const_cast<const ast::Nary*>(active_command.get()));

    PING();
  }

  std::string
  UConnection::make_prefix(const char* tag) const
  {
    std::ostringstream o;
    char fill = o.fill('0');
    o << '[' << std::setw(8) << server_.lastTime() / 1000L;
    o.fill(fill);
    if (tag && strlen(tag))
      o << ':' << tag;
    o << "] ";

    return o.str();
  }

  bool
  UConnection::has_pending_command() const
  {
    return shell_->pending_command_get();
  }

  void
  UConnection::drop_pending_commands()
  {
    shell_->pending_commands_clear();
  }

  bool
  UConnection::send_queue_empty() const
  {
    return send_queue_->empty();
  }

}
