/**
 ** \file object/task-class.hh
 ** \brief Definition of the URBI object task.
 */

#ifndef OBJECT_TASK_CLASS_HH
# define OBJECT_TASK_CLASS_HH

# include <scheduler/job.hh>

# include <object/fwd.hh>

namespace object
{
  extern rObject task_class;

  /// Create a task object from an existing job.
  rObject create_task_from_job(const scheduler::rJob&);

  /// Initialize the Task class.
  void task_class_initialize ();
}; // namespace object

#endif // !OBJECT_TASK_CLASS_HH
