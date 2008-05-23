/**
 ** \file runner/runner.hh
 ** \brief Definition of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HH
# define RUNNER_RUNNER_HH

# include <vector>

# include <boost/tuple/tuple.hpp>

# include "object/object.hh"
# include "scheduler/scheduler.hh"
# include "scheduler/job.hh"

namespace runner
{

  /// Ast executor.
  class Runner : public scheduler::Job
  {
  public:
    /// \name Useful shorthands.
    /// \{
    /// Super class type.
    typedef object::rObject rObject;
    typedef object::rLobby rLobby;
    /// \}

    /// \name Ctor & dtor.
    /// \{
    /// Construct a \c Runner in the \a lobby.  The runner needs to
    /// know its \a locals, who is its \a scheduler and will execute
    /// \a ast.  Memory ownership of \a ast is transferred to the Runner.
    /// The new runner has no parent.
    Runner(rLobby lobby, scheduler::Scheduler& scheduler);

    explicit Runner(const Runner&);

    /// Destroy a Runner.
    virtual ~Runner();
    /// \}

    /// \ name Accessors.
    /// \{
  public:
    /// Return the lobby in which this runner has been started.
    const rLobby& lobby_get() const;
    rLobby lobby_get();
    /// \}

    /// Send the current backtrace through the connection.
    /// \param chan The channel to print through.
    virtual void show_backtrace(const std::string& chan) = 0;

    void send_message_ (const std::string& tag, const std::string& msg);

    /// Execute the code of function \a func with arguments \a args in
    /// the local runner.
    ///
    /// \param func  the "function" to call: Primitive, Delegate, Code
    ///              or even rObject, in which case it is returned.
    ///
    /// \param msg   the name of the called method, as it should be stored
    ///              in a call message in case further processing is
    ///              required.
    ///
    /// \param args  the arguments.  Used as given, they are not evaluated
    ///              here.
    ///
    /// \param call_message  the callMessage.  Valid only for \a func
    ///             being Code.
    ///
    /// One cannot have both a call message and args.
    virtual rObject apply(const rObject& func,
			  const libport::Symbol msg,
			  object::objects_type args,
			  rObject call_message = 0) = 0;

    /// Use an argument list coming from Urbi.
    virtual rObject apply(const rObject& func, const libport::Symbol msg,
			  const object::rList& args) = 0;

  protected:
    /// \name Evaluation.
    /// \{

    /// Build a call message
    virtual rObject build_call_message(const rObject& tgt, const libport::Symbol& msg,
				       const object::objects_type& args) = 0;

    /// Do the actual work.  Implementation of \c Job::run.
    virtual void work() = 0;

    /// The URBI Lobby used to evaluate.
    /// Wraps an UConnection (ATM).
    rLobby lobby_;
  };

} // namespace runner

# include "runner/runner.hxx"

#endif // !RUNNER_RUNNER_HH
