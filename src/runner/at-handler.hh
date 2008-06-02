#ifndef RUNNER_AT_HANDLER_HH
#define RUNNER_AT_HANDLER_HH

# include "object/fwd.hh"
# include "runner/interpreter.hh"

namespace runner
{

  typedef object::rObject rObject;

  /// Register a new "at" job.
  ///
  /// \param starter The interpreter this job is started from.
  ///
  /// \param condition A lambda representing the expression to check.
  ///
  /// \param clause A lambda representing the code to run when the \a condition
  ///        becomes true, or nil. This lambda must start a detached task.
  ///
  /// \param con_leave A lambda representing the code to run when \a condition
  ///        becomes false, or nil. This lambda must start a detach task.
  void register_at_job(const runner::Interpreter& starter,
		       rObject condition,
		       rObject clause,
		       rObject on_leave);

} // namespace runner

#endif // RUNNER_AT_HANDLER_HH
