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

#include <runner/urbi-job.hh>

#include <eval/call.hh>
#include <eval/raise.hh>
#include <eval/send-message.hh>

#include <libport/config.h>

#ifndef LIBPORT_COMPILATION_MODE_SPEED
# include <runner/urbi-job.hxx>
#endif


namespace runner
{
  UrbiJob::~UrbiJob()
  {
  }


  /// Urbiscript evaluation
  /// \{

  void UrbiJob::work()
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
        libport::Finally finally(boost::bind(&UrbiJob::non_interruptible_set,
                                             this, non_interruptible_get()));
        non_interruptible_set(true);
        eval::show_exception(*this, exn);
      }
    }
  }

  void
  UrbiJob::scheduling_error(const std::string& msg)
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
        collector,
        "Scheduling");
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

  bool UrbiJob::frozen() const
  {
    return state.frozen();
  }

  size_t UrbiJob::has_tag(const sched::Tag& tag, size_t max_depth) const
  {
    return state.has_tag(tag, max_depth);
  }

  sched::prio_type UrbiJob::prio_get() const
  {
    return state.priority();
  }

  /// \}

  /// \name Profiling
  /// \{

  /*------------.
  | Profiling.  |
  `------------*/

  UrbiJob::Profile::Profile()
    : yields_(0)
    , wall_clock_time_(0)
    , total_time_(0)
    , function_calls_(0)
    , function_call_depth_max_(0)
    , checkpoint_(0)
    , function_call_depth_(0)
    , function_current_(0)
  {}


  UrbiJob::Profile::FunctionProfile::FunctionProfile()
    : calls_(0)
    , self_time_(0)
    , time_(0)
  {}

  UrbiJob::Profile::FunctionProfile&
  UrbiJob::Profile::FunctionProfile::operator+= (const FunctionProfile& rhs)
  {
    calls_     += rhs.calls_;
    self_time_ += rhs.self_time_;
    time_      += rhs.time_;
    return *this;
  }

  UrbiJob::Profile*
  UrbiJob::Profile::fork()
  {
    Profile* child = new Profile;
    child->function_call_depth_max_ = function_call_depth_max_;
    return child;
  }

  UrbiJob::Profile&
  UrbiJob::Profile::operator+=(const Profile& rhs)
  {
    yields_          += rhs.yields_;
    wall_clock_time_ += rhs.wall_clock_time_;
    total_time_      += rhs.total_time_;
    function_calls_  += rhs.function_calls_;

    function_call_depth_max_ = std::max(function_call_depth_max_,
                                        rhs.function_call_depth_max_);

    foreach(FunctionProfiles::value_type value, rhs.functions_profile_)
    {
      FunctionProfiles::iterator it = functions_profile_.find(value.first);
      if (it != functions_profile_.end())
        it->second += value.second;
      else
        functions_profile_.insert(value);
    }

    return *this;
  }

  void
  UrbiJob::Profile::join(Profile* other)
  {
    other->step();
    *this += *other;
    delete other;
  }

  UrbiJob::Profile::profile_idx
  UrbiJob::Profile::enter(void* function, libport::Symbol msg)
  {
    profile_idx prev = 0;
    if (wrapper_function_seen_)
    {
      step();
      prev = function_current_;
      function_current_ = function;
      ++function_calls_;
      ++function_call_depth_;
      if (function_call_depth_ > function_call_depth_max_)
        function_call_depth_max_ = function_call_depth_;
      ++functions_profile_[function].calls_;
      if (functions_profile_[function].name_.empty())
        functions_profile_[function].name_ = msg;
    }
    else
      wrapper_function_seen_ = true;
    return prev;
  }

  void
  UrbiJob::Profile::leave(profile_idx prev)
  {
    --function_call_depth_;
    step();
    function_current_ = prev;
  }

  libport::utime_t
  UrbiJob::Profile::step()
  {
    libport::utime_t now = libport::utime();
    libport::utime_t res = now - checkpoint_;
    total_time_ += res;
    wall_clock_time_ += res;
    functions_profile_[function_current_].self_time_ += res;
    checkpoint_ = now;
    return res;
  }

  void
  UrbiJob::profile_start(Profile* p)
  {
    assert(!profile);
    profile = p;
    profile->checkpoint_ = libport::utime();
    profile->functions_profile_[0].name_ = SYMBOL(LT_profiled_GT);
    ++profile->functions_profile_[0].calls_;
  }

  void
  UrbiJob::profile_stop()
  {
    assert(profile);
    profile->step();
    profile = 0;
  }

  void
  UrbiJob::profile_fork(UrbiJob& parent)
  {
    profile_start(parent.profile->fork());
  }

  void
  UrbiJob::profile_join(UrbiJob& child)
  {
    aver(profile);
    Profile* p = child.profile;
    child.profile_stop();
    profile->join(p);
  }

  void
  UrbiJob::hook_preempted() const
  {
    if (profile)
    {
      profile->step();
      ++profile->yields_;
    }
  }

  void
  UrbiJob::hook_resumed() const
  {
    if (profile)
    {
      libport::utime_t now = libport::utime();
      profile->wall_clock_time_ += now - profile->checkpoint_;
      profile->checkpoint_ = now;
    }
  }

  /// \}

  /// \name rest
  /// \{

  bool
  UrbiJob::non_interruptible_get() const
  {
    return super_type::non_interruptible_get();
  }

  void
  UrbiJob::non_interruptible_set(bool i)
  {
    super_type::non_interruptible_set(i);
  }

  /// \}

} // namespace runner
