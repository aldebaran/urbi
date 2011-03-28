/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file runner/state.hh
 ** \brief Interface for implementation of a tag stack.
 */

#ifndef RUNNER_STATE_HH
# define RUNNER_STATE_HH

// declare object::call_stack_type
# include <object/urbi-exception.hh>

// declare object::rTag & object::rLobby
# include <urbi/object/fwd.hh>

// declare sched::rTag
# include <sched/fwd.hh>

// declare sched::prio_type & sched::Tag
# include <sched/tag.hh>

// declare runner::Stacks
# include <runner/stacks.hh>

namespace runner
{
  /// TODO: This class handle all stacks visible in the Urbi world.  Later
  /// it should be rewritten into multiple interfaces and implementations to
  /// enable additional design of the Job states.
  class State
  {
  public:
    typedef object::Object Object;
    typedef object::rObject rObject;
    typedef object::Lobby Lobby;
    typedef object::rLobby rLobby;
    typedef object::rSlot rSlot;

  public:
    // FIXME: The lobby argument should be removed from this constructor,
    // but it is currently used to create the frame stack.
    /// Create a new empty stack.
    explicit State(rLobby lobby);

    /// Fork a process from another and reuse the stack of the forking
    /// process.
    explicit State(const State& base);

    /// Handle tags.
    /// \{

    /// Tag analyses
    /// \{
  public:

    /// Check if a tag is contained on the top of the stack.
    ///
    /// \param tag A scheduler tag, which is compared with object tags.
    /// \param max_depth The maximal depth of the visited tag stack.
    ///
    /// \return 0 if the job does not hold the tag,
    ///         the position of the tag in the tag stack
    ///         (starting from 1) otherwise.
    size_t has_tag(const sched::Tag& tag,
                   size_t max_depth = (size_t) - 1) const;

    /// Is one of the tag frozen?
    bool frozen() const;

    /// Get the highest tag priority.
    sched::prio_type priority() const;

  private:
    // This is supposed to be an optimization which is not one, because it
    // recompute the priority each time this is asked.
    void update_priority_cache(sched::prio_type prio);
    mutable bool priority_cache_valid_;
    mutable sched::prio_type priority_cache_;

    /// Whether the tag is frozen, even if no applied tag is frozen.  Do not
    /// use it's setter, prefer using the setter of the job, which notify
    /// the scheduler.
    ATTRIBUTE_Rw(bool, frozen);

    /// \}

  public:
    /// Stack of Urbi tags.
    typedef std::vector<object::rTag> tag_stack_type;

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
    tag_stack_type tag_stack_get() const;

    /// Clear the tag stack.
    void tag_stack_clear();

    /// Retrieve all the tags including the flow control tags.  It is only
    /// intended for internal usage.
    const tag_stack_type& tag_stack_get_all() const;

    /// Set the tag stack.
    void tag_stack_set(const tag_stack_type&);

    /// Return the size of the tag stack.
    size_t tag_stack_size() const;

  private:

    /// Push a tag on the tag stack.
    void tag_stack_push(const object::rTag& tag);

    /// Pop a tag from the tag stack.
    void tag_stack_pop();

    /// The stack of current tags. This is different from the
    /// sched::Tag stack located in the runner, that also stores
    /// C++ tags that are not visible in urbi. This stack is meant to
    /// enable to list current tags from Urbi.
    tag_stack_type tag_stack_;

    /// \}


    /// Scope tag
    /// \{
  public:

    /// Return the current scope_tag, after creating it if needed.
    const sched::rTag& scope_tag(sched::Scheduler& sched);

    /// Create dummy scope tag.
    void create_scope_tag();

    /// To be called at the end of a scope.
    void cleanup_scope_tag(sched::Scheduler& sched);

    /// Ditto, but may return 0 if there are no scope tag.
    const sched::rTag& scope_tag_get() const;

  private:
    /// The scope tags stack.
    std::vector<sched::rTag> scope_tags_;
    //// \}

    /// Call Stack
    /// \{
  public:
    // see urbi-exception.hh
    typedef object::call_type call_type;
    typedef object::call_stack_type call_stack_type;

    typedef rObject call_frame_type;
    typedef std::vector<call_frame_type> backtrace_type;

    const call_stack_type& call_stack_get() const;
    call_stack_type& call_stack_get();
    libport::Symbol innermost_call_get() const;

    /// Convert the current call_stack into a backtrace which contains for
    /// each call frame, a StackFrame object which contains the name of the
    /// method which is called and its location.
    backtrace_type backtrace_get() const;

  private:
    /// The call stack.
    call_stack_type call_stack_;
    /// \}

    /// Variable frame stack
    /// \{
  public:
    // This is a duplication of the interface of Stacks, but this is
    // temporary and should be modified as soon as the State can be
    // separated in multiple independent and/or inter-dependent components
    // to handle the state of a job.

    /// Type of a stack frame.
    typedef Stacks::frame_type var_frame_type;

    /// Type of a context.
    typedef Stacks::context_type var_context_type;

  /*--------------------------.
  | Starting / ending calls.  |
  `--------------------------*/

  public:
    /// Signal the stacks a new execution is starting
    void execution_starts(libport::Symbol msg);

    /// Push new frames on the stacks to execute a function
    /** \param msg      Name of the function being invoked (debug purpose only)
     *  \param local    Size of the local    variable frame.
     *  \param closed   Size of the closed   variable frame.
     *  \param captured Size of the captured variable frame.
     *  \param self     Value of 'this' in the new frame.
     *  \param call     Value of 'call' in the new frame.
     */
    var_frame_type
    push_frame(libport::Symbol msg,
               var_frame_type local_frame,
               rObject self, rObject call);

    /// Helper to restore a previous frame state
    void pop_frame(libport::Symbol msg, var_frame_type previous_frame);


    /// Push a whole new context. Returns a token for pop_context.
    var_context_type push_context(rObject self);
    /// Pop the context.
    void pop_context(const var_context_type& previous_context);

  /*-----------------.
  | Reading values.  |
  `-----------------*/

  public:
    /// Get value from the stack.
    rObject get(ast::rConstLocal e);
    /// Get slot from the stack.
    rSlot rget(ast::rConstLocal e);
    /// Get slot from the stack.
    rSlot
    rget_assignment(ast::rConstLocalAssignment e);
    /// Get 'this'.
    rObject this_get();
    /// Get 'call'.
    rObject call();

  /*-----------------.
  | Setting values.  |
  `-----------------*/

  public:
    /// Set 'this'.
    void this_set(rObject s);
    /// Set 'call'.
    void call_set(rObject v);
    /// Update the given value.
    void set(ast::rConstLocalAssignment e, rObject v);
    /// Define the given value.
    void def(ast::rConstLocalDeclaration e, rObject v,
             bool constant = false);
    /// Bind given argument.
    void def_arg(ast::rConstLocalDeclaration e, rObject v);
    /// Bind given captured variable.
    void def_captured(ast::rConstLocalDeclaration e, rSlot v);

    /// Switch the current 'this'.
    void this_switch(rObject s);


  private:
    /// The local variable stacks
    Stacks stacks_;

    /// \}

    /// Lobby
    /// \{
  public:

    // Don't return a rLobby, because the Lobby would be alive as long as
    // this stack is alive.
    Lobby* lobby_get();
    void lobby_set(Lobby* lobby);
    //Object* this_get();
    //void this_set(Object* this_);

  private:
    // Lobby in which the stack has been started.
    rLobby lobby_;
    //rObject this_;

    /// \}

    /// \ name Properties.
    /// \{
    /// Whether to err on redefinition or overwrite the slot.
    ATTRIBUTE_RW(bool, redefinition_mode);
    /// Whether to err if void is used as a value.
    ATTRIBUTE_RW(bool, void_error);
    /// \}

    /// \ name Last location
    /// \{
  public:
    void innermost_node_set(const ast::Ast* n);
    const ast::Ast* innermost_node_get() const;

  private:
    const ast::Ast* innermost_node_;
    /// \}

    /// \ name Last location
    /// \{
  public:
    /// The current exception when executing a "catch" block
    ATTRIBUTE_RW(rObject, current_exception);
    /// \}
  };


} // namespace runner

# if defined LIBPORT_COMPILATION_MODE_SPEED
#  include <runner/state.hxx>
# endif

#endif // ! RUNNER_STATE_HH
