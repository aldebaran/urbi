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

  void
  Runner::create_scope_tag()
  {
    scope_tags_.push_back(0);
  }

  scheduler::rTag
  Runner::scope_tag_get() const
  {
    return scope_tags_.back();
  }

  scheduler::rTag
  Runner::scope_tag()
  {
    scheduler::rTag tag = scope_tags_.back();
    if (!tag)
    {
      // Create the tag on demand.
      tag =
        new scheduler::Tag(libport::Symbol::fresh(SYMBOL(LT_scope_SP_tag_GT)));
      *scope_tags_.rbegin() = tag;
    }
    return tag;
  }

  void
  Runner::cleanup_scope_tag()
  {
    scheduler::rTag tag = scope_tags_.back();
    scope_tags_.pop_back();
    if (tag)
      tag->stop(scheduler_get(), object::void_class);
  }

} // namespace runner
