#ifndef RUNNER_SHELL_HH
# define RUNNER_SHELL_HH

# include <deque>

# include <libport/compiler.hh>
# include <libport/symbol.hh>

# include <ast/exps-type.hh>
# include <runner/interpreter.hh>

namespace runner
{

  class Shell : public Interpreter
  {
  public:
    Shell(const rLobby& lobby,
	  scheduler::Scheduler& scheduler,
	  const libport::Symbol& name = SYMBOL());
    ATTRIBUTE_NORETURN virtual void work();
    void append_command(const ast::rConstNary& command);
    bool pending_command_get() const;
    void pending_commands_clear();
  private:
    std::deque<ast::rConstNary> commands_;
    bool executing_;
  };

} // namespace runner

#endif // RUNNER_SHELL_HH
