#ifndef RUNNER_SHELL_HH
# define RUNNER_SHELL_HH

# include <deque>

# include <libport/symbol.hh>

# include <ast/exps-type.hh>
# include <runner/interpreter.hh>

namespace runner
{

  class Shell : public Interpreter
  {
  public:
    Shell(rLobby lobby,
	  scheduler::Scheduler& scheduler,
	  const libport::Symbol& name = SYMBOL());
    virtual void work();
    void append_command(ast::rConstNary command);
  private:
    std::deque<ast::rConstNary> commands_;
  };

} // namespace runner

#endif // RUNNER_SHELL_HH
