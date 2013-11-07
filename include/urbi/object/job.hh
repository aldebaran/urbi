/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file urbi/object/job.hh
 ** \brief Definition of the Urbi object job.
 */

#ifndef URBI_OBJECT_JOB_HH
# define URBI_OBJECT_JOB_HH

# include <urbi/object/cxx-object.hh>
# include <urbi/object/fwd.hh>
# include <urbi/object/list.hh>
# include <urbi/runner/fwd.hh>

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
      bool frozen() const;
      bool interruptible() const;
      void resetStats();
      const runner::tag_stack_type tags() const;
      void terminate();
      libport::ufloat timeShift() const;
      void waitForTermination();
      rObject lobby_get();
      void breakTag(int depth, rObject value);
    private:
      value_type value_;
    };

  }; // namespace object
}

#endif // !URBI_OBJECT_JOB_HH
