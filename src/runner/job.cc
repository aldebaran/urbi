/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/utime.hh>
#include <libport/finally.hh>

#include <object/profile.hh>
#include <urbi/object/slot.hh>
#include <urbi/object/string.hh>
#include <urbi/object/fwd.hh>

#include <runner/job.hh>

#include <eval/call.hh>
#include <eval/raise.hh>
#include <eval/send-message.hh>

#include <libport/config.h>

#ifndef LIBPORT_COMPILATION_MODE_SPEED
# include <runner/job.hxx>
#endif


namespace runner
{
  Job::~Job()
  {
  }


  /// Urbiscript evaluation
  /// \{

  void Job::work()
  {
    aver(worker_);
    try
    {
      result_cache_ = worker_(boost::ref(*this));
    }
    catch (object::UrbiException& exn)
    {
      // If this runner has a parent, let the exception go through
      // so that it will be handled by Job::run().
      if (child_job())
	throw;
      else
      // This is a detached runner, show the error.
      {
        // Yielding inside a catch is forbidden.
        libport::Finally finally(boost::bind(&Job::non_interruptible_set,
                                             this, non_interruptible_get()));
        non_interruptible_set(true);
        eval::show_exception(*this, exn);
      }
    }
  }

  void
  Job::scheduling_error(const std::string& msg)
  {
    // We may have a situation here. If the stack space is running
    // near exhaustion, we cannot reasonably hope that we will get
    // enough stack space to build an exception, which potentially
    // requires a non-negligible amount of calls. For this reason, we
    // create another job whose job is to build the exception (in a
    // freshly allocated stack) and propagate it to us as we are its
    // parent.
    //
    // Yet, if the user asked for non-interruptible mode, we do not
    // want to fire a new Runner...

    // The user requested non-interruptible mode for a reason, honor
    // it and try to throw in this job.
    if (non_interruptible_get())
      raise_scheduling_error(msg);

    CAPTURE_GLOBAL2(Exception, Scheduling);
    object::objects_type args;
    args << object::to_urbi(msg);
    sched::Job::Collector collector(this);
    sched::rJob child =
      spawn_child(
        eval::call(Scheduling->slot_get(SYMBOL(throwNew)), args),
        collector)
      ->name_set("Scheduling");
    child->start_job();

    try
    {
      yield_until_terminated(*child);
    }
    catch (const sched::ChildException& ce)
    {
      try
      {
	ce.rethrow_child_exception();
      }
      catch (const object::UrbiException& ue)
      {
	eval::raise(*this, ue.value_get(), false);
      }
    }
  }

  /// \}

  /// Job status defined by Tags
  /// \{

  bool Job::frozen() const
  {
    return state.frozen();
  }

  size_t Job::has_tag(const sched::Tag& tag, size_t max_depth) const
  {
    return state.has_tag(tag, max_depth);
  }

  sched::prio_type Job::prio_get() const
  {
    return state.priority();
  }

  /// \}

  Job* Job::name_set(const std::string& name)
  {
    rObject j = as_job();
    if (!j->local_slot_get(SYMBOL(name)))
      j->slot_set(SYMBOL(name), object::to_urbi(name));
    else
      j->slot_update(SYMBOL(name), object::to_urbi(name));
    return this;
  }

  const std::string
  Job::name_get() const
  {
    if (!terminated() && job_cache_)
      if (urbi::object::rSlot s =
          job_cache_->local_slot_get(SYMBOL(name)))
        return s->value()->as<object::String>()->value_get();
    return "Job";
  }

  /// \name Profiling
  /// \{

  /*------------.
  | Profiling.  |
  `------------*/

  void
  Job::profile_start(Profile* profile, libport::Symbol name,
                     Object* current, bool count)
  {
    assert(!profile_);
    assert(profile);
    profile_ = profile;
    profile_->start(name, current, count, profile_info_);
  }

  void
  Job::profile_stop()
  {
    assert(profile_);
    profile_->stop(profile_info_);
    profile_ = 0;
  }

  void
  Job::profile_fork(Job& parent)
  {
    profile_start(
      parent.profile_->fork(),
      libport::Symbol(),
      parent.profile_info_.function_current);
  }

  void
  Job::profile_join(Job& child)
  {
    aver(profile_);
    Profile* p = child.profile_;
    child.profile_stop();
    profile_->join(p);
  }

  Profile::idx
  Job::profile_enter(Object* function, libport::Symbol msg)
  {
    aver(profile_);
    return profile_->enter(function, msg, profile_info_);
  }

  void
  Job::profile_leave(Profile::idx idx)
  {
    aver(profile_);
    profile_->leave(idx, profile_info_);
  }

  void
  Job::hook_preempted() const
  {
    if (profile_)
      profile_->preempted(profile_info_);
  }

  void
  Job::hook_resumed() const
  {
    if (profile_)
      profile_->resumed(profile_info_);
  }

  /// \}

  /// \name rest
  /// \{

  bool
  Job::non_interruptible_get() const
  {
    return super_type::non_interruptible_get();
  }

  void
  Job::non_interruptible_set(bool i)
  {
    super_type::non_interruptible_set(i);
  }

  /// \}

} // namespace runner
