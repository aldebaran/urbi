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

#include <urbi/kernel/userver.hh>
#include <runner/exception.hh>
#include <urbi/runner/raise.hh>

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
  Exception::print(runner::Runner& r) const
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

  void Exception::Message::print(runner::Runner& r) const
  {
    std::string m = prefix.empty() ? msg
      : libport::format("%s: %s", prefix, msg);
    r.send_message(kind, libport::format("!!! %s: %s", loc, m));
    r.show_backtrace(kind);
  }
}
