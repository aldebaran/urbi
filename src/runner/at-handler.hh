/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#ifndef RUNNER_AT_HANDLER_HH
#define RUNNER_AT_HANDLER_HH

# include <object/fwd.hh>
# include <runner/interpreter.hh>

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
		       const rObject& condition,
		       const rObject& clause,
		       const rObject& on_leave);

} // namespace runner

#endif // RUNNER_AT_HANDLER_HH
