/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */

#include <boost/assign.hpp>

#include <kernel/uconnection.hh>
#include <runner/runner.hh>

namespace runner
{
  using namespace boost::assign;

  void
  Runner::send_message(const std::string& tag, const std::string& msg)
  {
    // If there is a Channel object with name 'tag', use it.
    rObject chan = lobby_->slot_locate(libport::Symbol(tag), true, true);
    if (chan && is_a(chan, lobby_->slot_get(SYMBOL(Channel))))
      apply(chan->slot_get(SYMBOL(LT_LT_)),
	    SYMBOL(LT_LT_),
	    list_of (chan)
	            (object::rString(new object::String(libport::Symbol(msg)))));
    else
      apply(lobby_->slot_get(SYMBOL(send)),
	    SYMBOL(send),
	    list_of (rObject(lobby_))
	            (new object::String(libport::Symbol(msg)))
	            (new object::String(libport::Symbol(tag))));
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
      // Create the tag on demand. It must have the lowest possible priority to
      // avoid influencing the scheduling algorithm.
      tag = new scheduler::Tag(libport::Symbol::fresh("<scope tag>"));
      tag->prio_set(scheduler_get(), scheduler::UPRIO_NONE);
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
