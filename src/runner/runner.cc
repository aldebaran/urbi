/*
 * Copyright (C) 2007-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */

#include <libport/containers.hh>
#include <libport/foreach.hh>
#include <libport/windows.hh>

#include <urbi/object/symbols.hh>

#include <urbi/object/job.hh>
#include <urbi/object/lobby.hh>
#include <urbi/object/slot.hh>
#include <urbi/object/tag.hh>

#include <runner/runner.hh>

namespace runner
{
  using object::rSlot;

  void
  Runner::send_message(const std::string& tag, const std::string& msg) const
  {
    // If there is a Channel object with name 'tag', use it.
    rSlot chan_slot = lobby_->slot_locate(libport::Symbol(tag), true).second;
    rObject chan = chan_slot ? chan_slot->value() : rObject();
    if (chan && is_a(chan, lobby_->slot_get(SYMBOL(Channel))))
      chan->call(SYMBOL(LT_LT_),
                 new object::String(msg));
    else
      lobby_->call(SYMBOL(send),
                   new object::String(msg),
                   new object::String(tag));
  }

  void
  Runner::create_scope_tag()
  {
    scope_tags_.push_back(0);
  }

  const sched::rTag&
  Runner::scope_tag_get() const
  {
    return scope_tags_.back();
  }

  const sched::rTag&
  Runner::scope_tag()
  {
    sched::rTag& tag = scope_tags_.back();
    if (!tag)
    {
      // Create the tag on demand. It must have the lowest possible priority to
      // avoid influencing the scheduling algorithm.
      tag = new sched::Tag(SYMBOL(scopeTag));
      tag->prio_set(scheduler_get(), sched::UPRIO_NONE);
    }
    return tag;
  }

  void
  Runner::cleanup_scope_tag()
  {
    const sched::rTag& tag = scope_tags_.back();
    if (tag)
      tag->stop(scheduler_get(), object::void_class);
    scope_tags_.pop_back();
  }

  void
  Runner::terminate_cleanup()
  {
    // Do not keep a reference on a job which keeps a reference onto
    // ourselves.
    job_ = 0;
    // Parent cleanup.
    super_type::terminate_cleanup();
  }

  object::rObject
  Runner::as_job()
  {
    if (terminated())
      return object::nil_class;
    if (!job_)
      job_ = new object::Job(this);
    return job_;
  }

  bool
  Runner::frozen() const
  {
    foreach (const object::rTag& tag, tag_stack_)
      if (tag->value_get()->frozen())
        return true;
    return frozen_;
  }

  void
  Runner::recompute_prio()
  {
    if (tag_stack_.empty())
    {
      prio_ = sched::UPRIO_DEFAULT;
      return;
    }
    prio_ = sched::UPRIO_MIN;
    foreach(const object::rTag& tag, tag_stack_)
      prio_ = std::max(prio_, tag->value_get()->prio_get());
  }

  void
  Runner::recompute_prio(sched::prio_type prio)
  {
    if (!scheduler_get().real_time_behavior_get())
      return;
    if (prio >= prio_ || tag_stack_.empty())
      recompute_prio();
  }

  size_t
  Runner::has_tag(const sched::Tag& tag, size_t max_depth) const
  {
    max_depth = std::min(max_depth, tag_stack_.size());
    for (size_t i = 0; i < max_depth; i++)
      if (tag_stack_[i]->value_get() == &tag)
	return i+1;
    return 0;
  }

  Runner::tag_stack_type
  Runner::tag_stack_get() const
  {
    tag_stack_type res;
    foreach (const object::rTag& tag, tag_stack_)
      if (!tag->value_get()->flow_control_get())
	res.push_back(tag);
    return res;
  }

  std::ostream&
  operator<<(std::ostream& o, const Runner::backtrace_type& b)
  {
    rforeach (const Runner::frame_type& c, b)
      o << "    called from: " << *c;
    return o;
  }

  void
  Runner::frozen_set(bool v)
  {
    frozen_ = v;
    scheduler_get().signal_world_change();
  }

  void
  Runner::dependency_add(object::rEvent evt)
  {
    assert(evt);
    if (dependencies_log_)
      dependencies_.insert(evt);
  }
} // namespace runner


#ifndef LIBPORT_COMPILATION_MODE_SPEED
# include <runner/runner.hxx>
#endif
