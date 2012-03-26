/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/containers.hh>

#include <runner/job.hh>

#include <urbi/kernel/userver.hh>
#include <runner/exception.hh>
#include <urbi/runner/raise.hh>

#include <eval/send-message.hh>

namespace runner
{
  Exception::Exception()
    : messages_()
  {}

  void
  Exception::err(const ast::loc& loc, const std::string& msg,
		 const std::string& prefix)
  {
    messages_ << Message(loc, "error", msg, prefix);
  }

  void
  Exception::warn(const ast::loc& loc, const std::string& msg,
		  const std::string& prefix)
  {
    // Since warning are non-fatal, we can print them directly and
    // carry on. To make them fatal, simply push the Message in
    // messages_ instead, like in Exception::err.
    Message(loc, "warning", msg, prefix).print(::kernel::runner());
  }

  bool
  Exception::empty() const
  {
    return messages_.empty();
  }

  void
  Exception::clear()
  {
    messages_.clear();
  }

  void
  Exception::print(runner::Job& r) const
  {
    foreach (const Message& m, messages_)
      m.print(r);
  }

  void
  Exception::raise(const std::string& input) const
  {
    aver(!messages_.empty());
    const Message& m = messages_.front();
    raise_syntax_error(m.loc, m.msg, input);
  }


  std::ostream&
  Exception::dump(std::ostream& o) const
  {
    foreach (const Message& m, messages_)
      m.dump(o) << std::endl;
    return o;
  }

  /*---------------------.
  | Exception::Message.  |
  `---------------------*/

  Exception::Message::Message(const ast::loc& loc,
                              const std::string& kind,
                              const std::string& msg,
                              const std::string& prefix)
    : loc(loc)
    , kind(kind)
    , msg(msg)
    , prefix(prefix)
  {}

  std::string
  Exception::Message::message() const
  {
    if (prefix.empty())
      return libport::format("!!! %s: %s", loc, msg);
    else
      return libport::format("!!! %s: %s: %s", loc, prefix, msg);
  }

  std::ostream&
  Exception::Message::dump(std::ostream& o) const
  {
    return o << "[" << kind << "] " << message();
  }

  void Exception::Message::print(runner::Job& r) const
  {
    eval::send_message(r, kind, message());
    eval::show_backtrace(r, kind);
  }

  std::ostream&
  operator<<(std::ostream& o, const Exception& e)
  {
    return e.dump(o);
  }
}
