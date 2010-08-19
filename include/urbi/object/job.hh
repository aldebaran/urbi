/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/job.hh
 ** \brief Definition of the Urbi object job.
 */

#ifndef OBJECT_JOB_HH
# define OBJECT_JOB_HH

# include <urbi/object/cxx-object.hh>
# include <urbi/object/fwd.hh>
# include <runner/runner.hh>
# include <sched/job.hh>

namespace urbi
{
  namespace object
  {
    class Job : public object::CxxObject
    {
    public:
      typedef sched::rJob value_type;

      Job(const value_type& value);
      Job(rJob model);
      const value_type& value_get() const;

      rList backtrace();
      libport::Symbol name();
      void setSideEffectFree(rObject);
      std::string status();
      const runner::tag_stack_type tags();
      void terminate();
      rFloat timeShift();
      void waitForChanges();
      void waitForTermination();

    private:
      value_type value_;

    URBI_CXX_OBJECT_(Job);
    };

  }; // namespace object
}

#endif // !OBJECT_JOB_HH
