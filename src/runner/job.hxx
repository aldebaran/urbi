/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef RUNNER_JOB_HXX
# define RUNNER_JOB_HXX

# include <libport/compilation.hh>

# include <urbi/object/job.hh>

// necesary for "dependencies_"
# include <urbi/object/event.hh>

namespace runner
{

  // Forking a child Job, implies having the same lobby and cloning the
  // state of the job, while creating a new working queue.
  LIBPORT_SPEED_INLINE
  Job::Job(const Job& model)
    : super_type(model)
    , profile_(0)
    , profile_info_()
    , dependencies_log_(false)
    , dependencies_()
    , state(model.state)
    , worker_()
    , result_cache_()
    , job_cache_()
  {
  }

  LIBPORT_SPEED_INLINE
  Job::Job(rLobby lobby,
           sched::Scheduler& scheduler)
    : super_type(scheduler)
    , profile_(0)
    , profile_info_()
    , dependencies_log_(false)
    , dependencies_()
    , state(lobby)
    , worker_()
    , result_cache_()
    , job_cache_()
  {
  }

  /// Urbiscript evaluation
  /// \{

  LIBPORT_SPEED_INLINE
  Job::rObject Job::result_get()
  {
    return result_cache_;
  }

  LIBPORT_SPEED_INLINE
  Job::rObject Job::as_job()
  {
    if (terminated())
      return object::nil_class;
    if (!job_cache_)
      // The following line increment the reference counter of this job,
      // thus we have to overload terminate_cleanup to remove this cycle.
      // Another possible solution, would be to artificially decrement our
      // own counter and increment it again in the destructor.
      job_cache_ = new object::Job(this);
    return job_cache_;
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void Job::terminate_cleanup()
  {
    // Do not keep a reference on a job which keeps a reference onto
    // ourselves.
    job_cache_ = 0;
    // Parent cleanup.
    super_type::terminate_cleanup();
  }

  /// \}

  /// Job status defined by Tags
  /// \{

  LIBPORT_SPEED_ALWAYS_INLINE
  void Job::frozen_set(bool v)
  {
    state.frozen_set(v);
    scheduler_get().signal_world_change();
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  bool Job::frozen_get()
  {
    return state.frozen_get();
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  bool Job::has_tag(const object::rTag& tag) const
  {
    return has_tag(*tag->value_get());
  }

  /// \}

  /// Job processing
  /// \{

  LIBPORT_SPEED_INLINE
  Job* Job::spawn_child(eval::Action action,
                        Job::Collector& collector)
  {
    Job* j = new Job(*this);

    j->set_action(action);
    register_child(j, collector);

    // We return a pointer, but an intrusive_pointer is kept by the
    // collector which has incremented its counter.
    return j;
  }

  LIBPORT_SPEED_INLINE
  Job* Job::spawn_child(eval::Action action)
  {
    Job* j = new Job(*this);
    j->set_action(action);
    return j;
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void Job::set_action(eval::Action action)
  {
    worker_ = action;
  }

  /// \}

  /// \name Profiling
  /// \{

  LIBPORT_SPEED_ALWAYS_INLINE
  bool Job::is_profiling() const
  {
    return profile_;
  }

  /// \}


  /// \name Dependencies tarcker
  /// \{

  LIBPORT_SPEED_ALWAYS_INLINE Job::dependencies_type&
  Job::dependencies()
  {
    return dependencies_;
  }

  LIBPORT_SPEED_ALWAYS_INLINE void
  Job::dependencies_log_set(bool v)
  {
    dependencies_log_ = v;
  }

  LIBPORT_SPEED_ALWAYS_INLINE bool
  Job::dependencies_log_get() const
  {
    return dependencies_log_;
  }

  LIBPORT_SPEED_INLINE void
  Job::dependency_add(object::rEvent evt)
  {
    assert(evt);
    if (dependencies_log_)
      dependencies_.insert(evt);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Job::dependencies_clear()
  {
    dependencies_.clear();
  }

  /// \}
}

#endif // ! RUNNER_JOB_HXX
