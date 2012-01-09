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
 ** \file runner/interpreter.hh
 ** \brief Definition of runner::Interpreter.
 */

#ifndef RUNNER_INTERPRETER_HH
# define RUNNER_INTERPRETER_HH

# include <boost/tuple/tuple.hpp>

# include <libport/compilation.hh>
# include <libport/finally.hh>
# include <libport/hash.hh>

# include <ast/fwd.hh>
# include <ast/exps-type.hh>

# include <urbi/object/urbi-exception.hh>

# include <runner/runner.hh>
# include <runner/stacks.hh>

namespace runner
{
  using urbi::object::Profile;
  using urbi::object::FunctionProfile;

  /// Ast executor.
  class Interpreter : public runner::Runner
  {
  public:
    /// \name Useful shorthands.
    /// \{
    /// Super class type.
    typedef runner::Runner super_type;
    typedef object::Object Object;
    typedef object::rObject rObject;
    typedef object::rSlot rSlot;
    typedef object::Code Code;
    typedef object::rCode rCode;
    typedef object::rLobby rLobby;
    typedef object::objects_type objects_type;
    typedef object::Slot Slot;
    /// \}

    /// \name Ctor & dtor.
    /// \{
    /// Construct a \c Interpreter in the \a lobby.  The runner needs
    /// to know its \a scheduler and will execute \a ast.  Memory
    /// ownership of \a ast is transferred to the Interpreter.  The
    /// new runner has no parent.
    Interpreter(rLobby lobby,
                sched::Scheduler& scheduler,
                ast::rConstAst ast,
                const std::string& name);

    Interpreter(const Interpreter&,
                rObject code,
                const std::string& name,
                const objects_type& args = objects_type());

    Interpreter(rLobby lobby,
                sched::Scheduler& sched,
                rObject code,
                const std::string& name,
                rObject self,
                const objects_type& args);

    Interpreter(rLobby lobby,
                sched::Scheduler& scheduler,
                boost::function0<void> job,
                rObject self,
                const std::string& name);

    /// Create a copy of a runner starting with another ast.
    Interpreter(const Interpreter&,
                ast::rConstAst ast,
                const std::string& name);

    /// Destroy a Interpreter.
    virtual ~Interpreter();
    /// \}

    /// Visit \a e within the current context.
    ATTRIBUTE_ALWAYS_INLINE object::rObject operator() (const ast::Ast* e);

    /// Evaluate \a e in a new context.
    object::rObject eval(const ast::Ast* e, rObject self);

    // Apply methods summary:
    //
    // Location in all apply methods is used to register the call
    // location in the call stack. It is optional because call
    // originating from C++ have no locations.
    //
    // * apply_ast(target, message, args)
    //
    // Call %target.%message(%args), args being given as ast
    // chunks. This enables to either evaluate the arguments, either
    // build a call message.
    //
    // * apply_ast(target, function, message, args)
    //
    // Same as above, but both target and function are specified. This
    // enables to call a method with another target than the holder of
    // the function.
    //
    // * apply(function, msg, args, call_message)
    //
    // Apply %function.  If the function is strict, you must give the
    // arguments in args, the first being the target.  If it is lazy,
    // you might either give the call message, either the arguments,
    // in which case a call message will be forged.  A call message
    // must be forged when a lazy function is called from C++ or with
    // eval: we only have the evaluated arguments.


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
    /// \param call_message  the callMessage.  Valid only when \a func
    ///             is a Code.
    ///
    virtual rObject apply(Object* func,
                          libport::Symbol msg,
                          const object::objects_type& args,
                          Object* call_message = 0);

    rObject apply(Object* function,
                  libport::Symbol msg,
                  const object::objects_type& args,
                  Object* call_message,
                  boost::optional<ast::loc> loc);

    virtual rObject apply_call_message(Object* function,
                                       libport::Symbol msg,
                                       Object* call_message);

    rObject apply_call_message(Object* function,
                               libport::Symbol msg,
                               Object* call_message,
                               boost::optional<ast::loc> loc);

    /// Invoke a function with the arguments as ast chunks.
    /// \param tgt   target (this).
    /// \param msg   name of the function to look up in tgt.
    /// \param args  effective argument.
    /// \param loc   call location to add the call stack.
    rObject apply_ast(Object* tgt,
                      libport::Symbol msg,
                      const ast::exps_type* args,
                      boost::optional<ast::loc> loc);

    /// Invoke a function with the arguments as ast chunks.
    /// \param tgt   target (this).
    /// \param fun   function to invoke
    /// \param msg   name under which the call is registered in the
    ///              stack
    /// \param args  effective argument.
    /// \param loc   call location to add the call stack.
    rObject apply_ast(Object* tgt,
                      Object* fun,
                      libport::Symbol msg,
                      const ast::exps_type* args,
                      boost::optional<ast::loc> loc);

    /// Return the result of the latest evaluation.
    virtual rObject result_get();

    /// Evaluate a tag and create it as well as the intermediate levels
    /// if needed.
    rObject eval_tag(ast::rConstExp);

    /// Make an urbi function from an ast chunk.
    object::rCode make_routine(ast::rConstRoutine f) const;

  protected:
    /// \name Evaluation.
    /// \{
    /// Build an evaluated arguments list containing \a tgt and
    /// arguments coming from \a args evaluated in the current context.
    /// Raise an error if any argument is void.
    void push_evaluated_arguments(object::objects_type& args,
                                  const ast::exps_type& ue_args);

    /// Build a call message
    virtual rObject
    build_call_message(Object* code,
		       libport::Symbol msg,
                       const object::objects_type& args);

    /// Build a call message
    rObject build_call_message(Object* tgt,
			       Object* code,
			       libport::Symbol msg,
                               const ast::exps_type& args);

  public:
    /// \name Profiling
    /// \{

    /// Start profiling into \a profile.
    void profile_start(Profile* profile, libport::Symbol name,
                       Object* current, bool count = false);
    /// Stop profiling.
    void profile_stop();
    /// Wheter there is a profiling in run.
    bool is_profiling() const;
    /// The current profile to fill - if any.
    ATTRIBUTE_R(Profile*, profile);
  protected:
    virtual void hook_preempted() const;
    virtual void hook_resumed() const;
  private:
    mutable libport::utime_t profile_checkpoint_;
    mutable Object* profile_function_current_;
    mutable unsigned profile_function_call_depth_;
    /// \}

  public:

#define VISIT(Macro, Data, Node)                                        \
    LIBPORT_SPEED_ALWAYS_INLINE object::rObject visit(const ast::Node* n);

    AST_FOR_EACH_NODE(VISIT);
#undef VISIT
    /// \}


    /// Do the actual work.  Implementation of \c Job::run.
    virtual void work();

    /// Signal a scheduling error exception.
    ///
    /// \param msg The explanation of the scheduling error.
    virtual void scheduling_error(const std::string& msg);

    virtual void show_backtrace(const std::string& chan) const;
    virtual backtrace_type backtrace_get() const;
    typedef object::call_stack_type call_stack_type;
    call_stack_type call_stack_get() const;

    /// Throw an excpetion.
    /**
     * \param exn Value to throw
     * \param skip_last Skip last call when reporting the backtrace
     */
    ATTRIBUTE_NORETURN
    virtual void raise(rObject exn, bool skip_last = false);
    ATTRIBUTE_NORETURN
    virtual void raise(rObject exn, bool skip_last,
                       const boost::optional<ast::loc>& loc);

    virtual libport::Symbol innermost_call_get() const;
    const ast::Ast* innermost_node() const;

    /// Report an exception.
    void show_exception(const object::UrbiException& ue,
                        const std::string& channel = "error") const;
  protected:

  private:
    void init();
    /// Reset result_, set the location and call stack of ue.
    rObject apply_urbi(Code* function,
                       libport::Symbol msg,
                       const object::objects_type& args,
                       Object* call_message);

  private:
    /// The root of the AST being executed.
    ast::rConstAst ast_;
    /// The urbi Code object to execute
    rObject code_;
    /// The job to execute
    boost::function0<void> job_;
    /// Its potential target
    rObject this_;
    /// Its arguments
    objects_type args_;

    /// The current value during the evaluation of the AST.
    rObject result_;

    /// The call stack.
    typedef object::call_type call_type;
    call_stack_type call_stack_;
    void show_backtrace(const call_stack_type& bt,
                        const std::string& chan) const;

    /// The local variable stacks
    Stacks stacks_;

    const ast::Ast* innermost_node_;

    /// The current exception when executing a "catch" block.
    rObject current_exception_;

    struct WatchEventData;
    static rObject watch_eval(WatchEventData* data);
    static void
    watch_run(WatchEventData* data,
                const object::objects_type& = object::objects_type());
    static void watch_stop(WatchEventData* data);
    static void watch_ward(WatchEventData* data);
    static void watch_unward(WatchEventData* data);

    static void
    at_run(WatchEventData* data,
           const object::objects_type& = object::objects_type());
    static void at_stop(WatchEventData* data);
    static void at_ward(WatchEventData* data);
    static void at_unward(WatchEventData* data);


    // Work around limitations of VC++ 2005.
#define FINALLY_Do(DefineOrUse)                 \
    FINALLY_ ## DefineOrUse                     \
    (Do,                                        \
     ((Stacks&, stacks_))                       \
     ((rObject&, old_tgt)),                     \
     stacks_.this_switch(old_tgt))

#define FINALLY_Scope(DefineOrUse)                      \
    FINALLY_ ## DefineOrUse                             \
    (Scope,                                             \
     ((Interpreter*, i))                                \
     ((bool&, non_interruptible_))                      \
     ((bool, non_interruptible))                        \
     ((bool, redefinition_mode))                        \
     ((bool, void_error)),                              \
     i->cleanup_scope_tag();                            \
     non_interruptible_ = non_interruptible;            \
     i->redefinition_mode_set(redefinition_mode);       \
     i->void_error_set(void_error);                     \
      )

#define FINALLY_Try(DefineOrUse)                \
    FINALLY_ ## DefineOrUse                     \
    (Try,                                       \
     ((rObject&, current_exception_))           \
     ((rObject&, old_exception)),               \
     current_exception_ = old_exception)

    FINALLY_Do(DEFINE);
    FINALLY_Scope(DEFINE);
    FINALLY_Try(DEFINE);

  };

} // namespace runner

# include <runner/interpreter.hxx>

#endif // !RUNNER_INTERPRETER_HH
