/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
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

# include <parser/uparser.hh>

namespace runner
{

  class Shell : public Interpreter
  {
  public:
    Shell(const rLobby& lobby,
	  sched::Scheduler& scheduler,
	  const std::string& name,
          std::istream& input);
    ~Shell();
    virtual void work();
    void work_();
    bool pending_command_get() const;
    void pending_commands_clear();

    void processSerializedMessages();
    /** Enable/disable serialization mode. If set, expects serialized messages.
     * Sends a reply message on tag 'tagReply'.
     * This function does not return, and must be called from the Shell itself.
     * (i.e. the remote must call setSerializationMode() at toplevel.
     */
    void setSerializationMode(bool, const std::string& tagReply);
    /// Notify the shell that it must stop.
    void stop();
  private:

    /// Evaluate \a exp and print its value.
    /// \precondition \a exp should be a foreground job.
    void eval_print_(const ast::Exp* exp);
    /// Execute the front of commands_.
    void handle_command_(ast::rConstExp exp, bool canYield = true);
    /// Handle end of a command.
    void handle_command_end_();

    std::deque<ast::rConstExp> commands_;
    bool executing_;
    std::istream& input_;
    bool stop_;
    typedef sched::jobs_type jobs_type;
    jobs_type jobs_;
    // Expect serialized messages.
    bool binary_mode_;
    // Job handling serialized message.
    sched::rJob serializationJob_;
    // Our parser, deleted when we switch to binary mode.
    parser::UParser* parser_;
  };

} // namespace runner

#endif // RUNNER_SHELL_HH
