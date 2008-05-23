/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */

#include <kernel/uconnection.hh>
#include <runner/runner.hh>
#include <sstream>
#include <object/atom.hh>

namespace runner
{
  void
  Runner::send_message_ (const std::string& tag, const std::string& msg)
  {
    UConnection& c = lobby_->value_get().connection;
    c.send (msg.c_str(), tag.c_str());
    c.endline();
  }

} // namespace runner
