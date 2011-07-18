/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
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

// FIXME: This link to headers which are not installed.
# include <runner/job.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Job : public object::CxxObject
    {
      URBI_CXX_OBJECT(Job, CxxObject);
    public:
      typedef runner::rJob value_type;

      Job(const value_type& value);
      Job(rJob model);
      const value_type& value_get() const;

      /// The runner.
      static object::rJob current();
      /// All the running jobs.
      static object::List::value_type jobs();

      rList backtrace() const;
      const std::string& name() const;
      std::string status() const;
      rObject stats() const;
      void resetStats();
      const runner::State::tag_stack_type tags() const;
      void terminate();
      libport::ufloat timeShift() const;
      void waitForChanges();
      void waitForTermination();

    private:
      value_type value_;
    };

  }; // namespace object
}

#endif // !OBJECT_JOB_HH
