/**
 ** \file object/task-class.hh
 ** \brief Definition of the URBI object task.
 */

#ifndef OBJECT_TASK_CLASS_HH
# define OBJECT_TASK_CLASS_HH

# include <object/cxx-object.hh>
# include <object/fwd.hh>
# include <runner/runner.hh>
# include <scheduler/job.hh>

namespace object
{
  class Task : public object::CxxObject
  {
  public:
    typedef scheduler::rJob value_type;

    Task();
    Task(const value_type& value);
    Task(rTask model);
    const value_type& value_get() const;

    rList backtrace();
    libport::Symbol name();
    void setSideEffectFree(rObject);
    std::string status(runner::Runner&);
    const runner::tag_stack_type& tags();
    void terminate();
    rFloat timeShift();
    void waitForChanges(runner::Runner&);
    void waitForTermination(runner::Runner&);

    static void initialize(CxxObject::Binder<Task>& bind);
    static const std::string type_name;
    static bool task_added;
    virtual std::string type_name_get() const;
    static rObject proto;

  private:
    value_type value_;
  };

}; // namespace object

#endif // !OBJECT_TASK_CLASS_HH
