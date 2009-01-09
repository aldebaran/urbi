/**
 ** \file runner/runner.hh
 ** \brief Definition of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HH
# define RUNNER_RUNNER_HH

# include <vector>

# include <boost/tuple/tuple.hpp>

# include <libport/compiler.hh>

# include <object/fwd.hh>
# include <object/list.hh>
# include <sched/scheduler.hh>
# include <sched/job.hh>

namespace runner
{

  /// Stack of Urbi tags.
  typedef std::vector<object::rTag> tag_stack_type;

  /// Ast executor.
  class Runner : public scheduler::Job
  {
  public:
    /// \name Useful shorthands.
    /// \{
    /// Super class type.
    typedef scheduler::Job super_type;

    /// Shorthands used everywhere.
    typedef object::rObject rObject;
    typedef object::rLobby rLobby;
    /// \}

    /// \name Ctor & dtor.
    /// \{
    /// Construct a \c Runner in the \a lobby.  The runner needs to
    /// know its \a locals, who is its \a scheduler and will execute
    /// \a ast.  Memory ownership of \a ast is transferred to the Runner.
    /// The new runner has no parent.
    Runner(rLobby lobby,
	   scheduler::Scheduler& scheduler,
	   const libport::Symbol& name);

    explicit Runner(const Runner&, const libport::Symbol& name);

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

    /// Change lobby on fly
    void lobby_set(rLobby);

    /// Return the result of the latest evaluation.
    virtual rObject result_get() = 0;

    /// Send the current backtrace through the connection.
    /// \param chan The channel to print through.
    virtual void show_backtrace(const std::string& chan) = 0;
    /// Retreive the current backtrace as a list of (function name,
    /// location) pairs
    typedef std::pair<std::string, std::string> frame_type;
    typedef std::vector<frame_type> backtrace_type;
    virtual backtrace_type backtrace_get() const = 0;

    virtual void send_message(const std::string& tag, const std::string& msg);

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
    virtual rObject apply(const rObject& target,
                          const rObject& function,
			  const libport::Symbol msg,
			  object::objects_type& args,
			  const rObject& call_message = 0) = 0;

    virtual rObject apply_call_message(const rObject& function,
                                       const libport::Symbol msg,
                                       const rObject& call_message) = 0;

    /// Return the current scope_tag, after creating it if needed.
    const scheduler::rTag& scope_tag();

    /// Get the current runner as an Urbi Task or nil if the task is
    /// terminated.
    rObject as_task();

    ATTRIBUTE_NORETURN
    virtual void raise(rObject exn, bool skip_last = false) = 0;
    virtual libport::Symbol innermost_call_get() const = 0;

    virtual bool frozen() const;
    virtual size_t has_tag(const scheduler::Tag& tag,
			   size_t max_depth = (size_t)-1) const;
    bool has_tag(const object::rTag& tag) const;

    virtual scheduler::prio_type prio_get() const;

    /// Apply a tag to the current job tag stack.
    ///
    /// \param tag The tag to apply.
    ///
    /// \param finally The action executed when the
    ///                tag is removed. No action is
    ///                inserted if 0 is given.
    void apply_tag(const object::rTag& tag, libport::Finally* finally);

    /// Retrieve the tags currently tagging the runned code.
    /// This does not include the flow control tags and is only
    /// intended for user consumption.
    tag_stack_type tag_stack_get() const;

    /// Clear the tag stack.
    void tag_stack_clear();

  protected:

    /// Set the tag stack.
    void tag_stack_set(const tag_stack_type&);

    /// Return the size of the tag stack.
    size_t tag_stack_size() const;

  protected:
    /// \name Evaluation.
    /// \{

    /// Build a call message
    /**
     *  \a args is taken by reference for performance sake, and might
     *  be arbitrarily modified by apply!
     */
    virtual rObject build_call_message(const rObject& tgt,
				       const rObject& code,
				       const libport::Symbol& msg,
				       const object::objects_type& args) = 0;

    // Ensure proper cleanup.
    virtual void terminate_cleanup();

    /// Create dummy scope tag.
    void create_scope_tag();

    /// To be called at the end of a scope.
    void cleanup_scope_tag();

    /// Ditto, but may return 0 if there are no scope tag.
    const scheduler::rTag& scope_tag_get() const;

    /// The URBI Lobby used to evaluate.
    /// Wraps an UConnection (ATM).
    rLobby lobby_;

  private:
    /// Recompute the current priority after a tag operation.
    void recompute_prio();

    /// Recompute the current priority if a particular tag could have
    /// affected it (addition or removal).
    void recompute_prio(scheduler::prio_type prio);

    /// The scope tags stack.
    std::vector<scheduler::rTag> scope_tags_;

    /// The stack of current tags. This is different from the
    /// scheduler::Tag stack located in the runner, that also stores
    /// C++ tags that are not visible in urbi. This stack is meant to
    /// enable to list current tags from Urbi.
    tag_stack_type tag_stack_;

    /// The runner seen as an Urbi Task.
    object::rTask task_;

    /// Current priority.
    scheduler::prio_type prio_;
  };

} // namespace runner

# ifdef LIBPORT_SPEED
#  include <runner/runner.hxx>
# endif

#endif // !RUNNER_RUNNER_HH
