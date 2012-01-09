/*
 * Copyright (C) 2007-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file runner/runner.hh
 ** \brief Definition of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HH
# define RUNNER_RUNNER_HH

# include <vector>

# include <boost/tuple/tuple.hpp>
# include <boost/unordered_set.hpp>

# include <libport/attributes.hh>
# include <libport/compiler.hh>
# include <libport/config.h>

# include <sched/scheduler.hh>
# include <sched/job.hh>

# include <urbi/object/fwd.hh>

# include <ast/loc.hh>

namespace runner
{

  /// Ast executor.
  class Runner : public sched::Job
  {
  public:
    /// \name Useful shorthands.
    /// \{
    /// Super class type.
    typedef sched::Job super_type;

    /// Shorthands used everywhere.
    typedef object::Object Object;
    typedef object::rObject rObject;
    typedef object::rLobby rLobby;
    /// \}

    /// \name Ctor & dtor.
    /// \{
    /// Construct a \c Runner in the \a lobby.  The runner needs to
    /// know its \a scheduler.  The new runner has no parent.
    Runner(rLobby lobby,
           sched::Scheduler& scheduler,
           const std::string& name);

    explicit Runner(const Runner&, const std::string& name);

    /// Destroy a Runner.
    virtual ~Runner();
    /// \}

    /// \ name Accessors.
    /// \{
  public:
    /// Return the lobby in which this runner has been started.
    rLobby lobby_get() const;
    rLobby lobby_get();
    /// \}

    /// \ name Properties.
    /// \{
    /// Whether to err on redefinition or overwrite the slot.
    ATTRIBUTE_RW(bool, redefinition_mode);
    /// Whether to err if void is used as a value.
    ATTRIBUTE_RW(bool, void_error);
    /// \}

  public:
    /// Change lobby on fly
    void lobby_set(rLobby);

    /// Return the result of the latest evaluation.
    virtual rObject result_get() = 0;

    /// Send the current backtrace through the connection.
    /// \param chan The channel to print through.
    virtual void show_backtrace(const std::string& chan) const = 0;

    /// Retreive the current backtrace as a list of (function name,
    /// location) pairs
    typedef rObject frame_type;
    typedef std::vector<frame_type> backtrace_type;
    virtual backtrace_type backtrace_get() const = 0;

    virtual
    void send_message(const std::string& tag, const std::string& msg) const;

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
    virtual rObject apply(Object* function,
			  libport::Symbol msg,
			  const object::objects_type& args,
			  Object* call_message = 0) = 0;

    virtual rObject apply_call_message(Object* function,
                                       libport::Symbol msg,
                                       Object* call_message) = 0;
    virtual rObject apply_call_message(Object* function,
                                       libport::Symbol msg,
                                       Object* call_message,
                                       boost::optional<ast::loc> loc) = 0;

    /// Return the current scope_tag, after creating it if needed.
    const sched::rTag& scope_tag();

    /// Get the current runner as an Urbi Job or nil if the job is
    /// terminated.
    rObject as_job();

    ATTRIBUTE_NORETURN
    virtual void raise(rObject exn, bool skip_last = false) = 0;
    virtual libport::Symbol innermost_call_get() const = 0;

    virtual bool frozen() const;
    virtual size_t has_tag(const sched::Tag& tag,
			   size_t max_depth = (size_t)-1) const;
    bool has_tag(const object::rTag& tag) const;

    virtual sched::prio_type prio_get() const;

    /// Apply a tag to the current job tag stack.
    ///
    /// \param tag The tag to apply.
    ///
    /// \param finally The action executed when the
    ///                tag is removed. No action is
    ///                inserted if 0 is given.
    void apply_tag(const object::rTag& tag, libport::Finally* finally = 0);

    /// Retrieve the tags currently tagging the runned code.
    /// This does not include the flow control tags and is only
    /// intended for user consumption.
    typedef urbi::object::tag_stack_type tag_stack_type;
    tag_stack_type tag_stack_get() const;

    /// Clear the tag stack.
    void tag_stack_clear();

    /// Retrieve all the tags including the flow control tags.  It is only
    /// intended for internal usage.
    const tag_stack_type& tag_stack_get_all() const;

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
    virtual rObject build_call_message(Object* code,
				       libport::Symbol msg,
				       const object::objects_type& args) = 0;

    // Ensure proper cleanup.
    virtual void terminate_cleanup();

    /// Create dummy scope tag.
    void create_scope_tag();

    /// To be called at the end of a scope.
    void cleanup_scope_tag();

    /// Ditto, but may return 0 if there are no scope tag.
    const sched::rTag& scope_tag_get() const;

    /// The Urbi Lobby used to evaluate.
    /// Wraps an UConnection (ATM).
    rLobby lobby_;

  private:
    /// Recompute the current priority after a tag operation.
    void recompute_prio();

    /// Recompute the current priority if a particular tag could have
    /// affected it (addition or removal).
    void recompute_prio(sched::prio_type prio);

    /// The scope tags stack.
    std::vector<sched::rTag> scope_tags_;

    /// The stack of current tags. This is different from the
    /// sched::Tag stack located in the runner, that also stores
    /// C++ tags that are not visible in urbi. This stack is meant to
    /// enable to list current tags from Urbi.
    tag_stack_type tag_stack_;

    /// The runner seen as an Urbi Job.
    object::rJob job_;

    /// Current priority.
    sched::prio_type prio_;

    /// Whether the tag is frozen, even if no applied tag is frozen.
    ATTRIBUTE_Rw(bool, frozen);

  public:
    typedef boost::unordered_set<object::rEvent> dependencies_type;
    const dependencies_type& dependencies() const;
    bool dependencies_log_get() const;
    void dependencies_log_set(bool val);
    void dependency_add(object::rEvent evt);
  protected:
    bool dependencies_log_;
    dependencies_type dependencies_;
  };

  /// Dump \a b on \a o, for debugging.
  std::ostream& operator<<(std::ostream& o, const Runner::backtrace_type& b);

  /// Smart pointer shorthand
  typedef libport::intrusive_ptr<Runner> rRunner;

} // namespace runner

# ifdef LIBPORT_COMPILATION_MODE_SPEED
#  include <runner/runner.hxx>
# endif

#endif // !RUNNER_RUNNER_HH
