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
# include <istream>

# include <libport/compiler.hh>
# include <libport/symbol.hh>

# include <sched/job.hh>

# include <ast/exps-type.hh>
# include <runner/interpreter.hh>

namespace runner
{

  class Shell : public Interpreter
  {
  public:
    Shell(const rLobby& lobby,
	  sched::Scheduler& scheduler,
	  libport::Symbol name,
          std::istream& input);
    virtual void work();
    void work_();
    bool pending_command_get() const;
    void pending_commands_clear();

  private:

    /// Evaluate \a exp and print its value.
    /// \precondition \a exp should be a foreground job.
    void eval_print_(const ast::Exp* exp);
    /// Execute the front of commands_.
    void handle_command_(ast::rConstExp exp);
    std::deque<ast::rConstExp> commands_;
    bool executing_;
    std::istream& input_;
    bool stop_;
    typedef sched::jobs_type jobs_type;
    jobs_type jobs_;
  };

} // namespace runner

#endif // RUNNER_SHELL_HH
