/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

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
	  sched::Scheduler& scheduler,
	  libport::Symbol name);
    ATTRIBUTE_NORETURN virtual void work();
    void work_();
    void append_command(const ast::rConstNary& command);
    void insert_oob_call(boost::function0<void> func);
    bool pending_command_get() const;
    void pending_commands_clear();

  private:

    /// Evaluate \a exp and print its value.
    /// \precondition \a exp should be a foreground job.
    void eval_print_(const ast::Exp* exp);
    /// Execute the front of commands_.
    void handle_command_();
    /// Execute everything in oob_calls_.
    void handle_oob_();
    std::deque<ast::rConstExp> commands_;
    std::list<boost::function0<void> > oob_calls_;
    bool executing_;
  };

} // namespace runner

#endif // RUNNER_SHELL_HH
