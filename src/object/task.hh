/**
 ** \file object/task-class.hh
 ** \brief Definition of the URBI object task.
 */

#ifndef OBJECT_TASK_CLASS_HH
# define OBJECT_TASK_CLASS_HH

# include <object/cxx-object.hh>
# include <object/fwd.hh>
# include <runner/runner.hh>
# include <sched/job.hh>

namespace object
{
  class Task : public object::CxxObject
  {
  public:
    typedef sched::rJob value_type;

    Task();
    Task(const value_type& value);
    Task(rTask model);
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

  URBI_CXX_OBJECT(Task);
  };

}; // namespace object

#endif // !OBJECT_TASK_CLASS_HH
