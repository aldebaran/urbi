/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */

#include <kernel/uconnection.hh>
#include <object/atom.hh>
#include <runner/runner.hh>

namespace runner
{
  void
  Runner::send_message(const std::string& tag, const std::string& msg)
  {
    object::objects_type args;
    args.push_back(lobby_);
    args.push_back(object::rString(new object::String(libport::Symbol(msg))));
    args.push_back(object::rString(new object::String(libport::Symbol(tag))));
    apply(lobby_->slot_get(SYMBOL(send)), SYMBOL(send), args);
    //UConnection& c = lobby_->value_get().connection;
    //c.send(msg.c_str(), msg.size(), tag.c_str());
  }

} // namespace runner
