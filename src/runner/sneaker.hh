#ifndef RUNNER_SNEAKER_HH
# define RUNNER_SNEAKER_HH

# include <ast/ast.hh>
# include <object/primitives.hh>
# include <runner/interpreter.hh>
# include <scheduler/scheduler.hh>

namespace dbg
{

  /// Create the sneaker if it hasn't been created yet.
  ///
  /// \param lobby     Any lobby.
  ///
  /// \param scheduler The current scheduler.
  void create_sneaker_if_needed
    (object::rLobby lobby, scheduler::Scheduler& scheduler);

  /// Retrieve the current runner or the sneaker. This should be used
  /// only when debugging, when we really need a runner of some kind
  /// to execute code.
  ///
  /// \return A runner.
  runner::Runner& runner_or_sneaker_get();

  /// The following functions will be called from the debugger. Here
  /// are some example uses:
  ///
  ///        (gdb) call dbg::dump(object::system_class, 1)
  ///
  ///        (gdb) call dbg::eval("System")
  ///
  ///        (gdb) call dbg::evalp("6*7")
  ///
  ///        (gdb) call dbg::ps()

  void dump(const object::rObject& o, int depth);
  object::rObject eval(const char* command);
  void evalp(const char* command);
  void ps();
  void pp(ast::rAst);

} // namespace dbg

#endif // RUNNER_SNEAKER_HH
