/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */

#include <libport/containers.hh>
#include <libport/windows.hh>

#include <kernel/uconnection.hh>

#include <object/lobby.hh>
#include <object/task.hh>

#include <runner/runner.hh>

namespace runner
{
  void
  Runner::send_message(const std::string& tag, const std::string& msg)
  {
    // If there is a Channel object with name 'tag', use it.
    rObject chan = lobby_->slot_locate(libport::Symbol(tag), true, true);
    object::objects_type args;
    if (chan && is_a(chan, lobby_->slot_get(SYMBOL(Channel))))
    {
      args.push_back(new object::String(msg));
      apply(chan,
            chan->slot_get(SYMBOL(LT_LT_)),
	    SYMBOL(LT_LT_),
	    args);
    }
    else
    {
      args.push_back(new object::String(msg));
      args.push_back(new object::String(tag));
      apply(lobby_,
            lobby_->slot_get(SYMBOL(send)),
	    SYMBOL(send),
            args);
    }
  }

  void
  Runner::create_scope_tag()
  {
    scope_tags_.push_back(0);
  }

  const scheduler::rTag&
  Runner::scope_tag_get() const
  {
    return scope_tags_.back();
  }

  const scheduler::rTag&
  Runner::scope_tag()
  {
    scheduler::rTag& tag = scope_tags_.back();
    if (!tag)
    {
      // Create the tag on demand. It must have the lowest possible priority to
      // avoid influencing the scheduling algorithm.
      tag = new scheduler::Tag(SYMBOL(scopeTag));
      tag->prio_set(scheduler_get(), scheduler::UPRIO_NONE);
    }
    return tag;
  }

  void
  Runner::cleanup_scope_tag()
  {
    const scheduler::rTag& tag = scope_tags_.back();
    if (tag)
      tag->stop(scheduler_get(), object::void_class);
    scope_tags_.pop_back();
  }

  void
  Runner::terminate_cleanup()
  {
    // Do not keep a reference on a task which keeps a reference onto ourselves.
    task_ = 0;
    // Parent cleanup.
    super_type::terminate_cleanup();
  }

  object::rObject
  Runner::as_task()
  {
    if (terminated())
      return object::nil_class;
    if (!task_)
      task_ = new object::Task(this);
    return task_;
  }

  bool
  Runner::frozen() const
  {
    return libport::has_if(tags_, boost::mem_fn(&scheduler::Tag::frozen));
  }

  void
  Runner::recompute_prio()
  {
    if (tags_.empty())
    {
      prio_ = scheduler::UPRIO_DEFAULT;
      return;
    }
    prio_ = scheduler::UPRIO_MIN;
    foreach(const scheduler::rTag& tag, tags_)
      prio_ = std::max(prio_, tag->prio_get());
  }

  void
  Runner::recompute_prio(const scheduler::Tag& tag)
  {
    if (tag.prio_get() >= prio_ || tags_.empty())
      recompute_prio();
  }

  size_t
  Runner::has_tag(const scheduler::Tag& tag, size_t max_depth) const
  {
    max_depth = std::min(max_depth, tags_.size());
    for (size_t i = 0; i < max_depth; i++)
      if (tags_[i]->derives_from(tag))
	return i+1;
    return 0;
  }

} // namespace runner


#ifndef LIBPORT_SPEED
# include <runner/runner.hxx>
#endif
