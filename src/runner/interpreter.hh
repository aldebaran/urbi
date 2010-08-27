/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
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

# include <object/urbi-exception.hh>

# include <runner/runner.hh>
# include <runner/stacks.hh>

namespace runner
{

  /// Ast executor.
  class Interpreter : public runner::Runner
  {
  public:
    /// \name Useful shorthands.
    /// \{
    /// Super class type.
    typedef runner::Runner super_type;
    typedef object::rObject rObject;
    typedef object::rSlot rSlot;
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
                libport::Symbol name);

    Interpreter(const Interpreter&,
                rObject code,
                libport::Symbol name,
                const objects_type& args = objects_type());

    Interpreter(rLobby lobby,
                sched::Scheduler& sched,
                rObject code,
                libport::Symbol name,
                rObject self,
                const objects_type& args);

    Interpreter(rLobby lobby,
                sched::Scheduler& scheduler,
                boost::function0<void> job,
                rObject self,
                libport::Symbol name);

    /// Create a copy of a runner starting with another ast.
    Interpreter(const Interpreter&,
                ast::rConstAst ast,
                libport::Symbol name);

    /// Destroy a Interpreter.
    virtual ~Interpreter();
    /// \}

    /// The entry point: visit \a e.
    ATTRIBUTE_ALWAYS_INLINE object::rObject operator() (const ast::Ast* e);

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
    virtual rObject apply(const rObject& func,
                          libport::Symbol msg,
                          const object::objects_type& args,
                          const rObject& call_message = 0);

    rObject apply(const rObject& function,
                  libport::Symbol msg,
                  const object::objects_type& args,
                  const rObject& call_message,
                  boost::optional<ast::loc> loc);

    virtual rObject apply_call_message(const rObject& function,
                                       libport::Symbol msg,
                                       const rObject& call_message);

    rObject apply_call_message(const rObject& function,
                               libport::Symbol msg,
                               const rObject& call_message,
                               boost::optional<ast::loc> loc);

    /// Invoke a function with the arguments as ast chunks.
    /// \param tgt   target (this).
    /// \param msg   name of the function to look up in tgt.
    /// \param args  effective argument.
    /// \param loc   call location to add the call stack.
    rObject apply_ast(const rObject& tgt,
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
    rObject apply_ast(const rObject& tgt,
                      const rObject& fun,
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
    build_call_message(const rObject& code,
		       libport::Symbol msg,
                       const object::objects_type& args);

    /// Build a call message
    rObject build_call_message(const rObject& tgt,
			       const rObject& code,
			       libport::Symbol msg,
                               const ast::exps_type& args);

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
    object::call_stack_type call_stack_get() const;

    /// Throw an excpetion.
    /**
     * \param exn Value to throw
     * \param skip_last Skip last call when reporting the backtrace
     */
    ATTRIBUTE_NORETURN
    virtual void raise(rObject exn, bool skip_last = false);

    virtual libport::Symbol innermost_call_get() const;

    /// Report an exception.
    void show_exception(const object::UrbiException& ue,
                        const std::string& channel = "error") const;
  protected:

  private:
    void init();
    /// Reset result_, set the location and call stack of ue.
    rObject apply_urbi(const rCode& function,
                       libport::Symbol msg,
                       const object::objects_type& args,
                       const rObject& call_message);

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
    typedef object::call_stack_type call_stack_type;
    typedef object::call_type call_type;
    call_stack_type call_stack_;
    void show_backtrace(const call_stack_type& bt,
                        const std::string& chan) const;

    /// The local variable stacks
    Stacks stacks_;

    const ast::Ast* innermost_node_;

    /// The current exception when executing a "catch" block.
    rObject current_exception_;

    struct AtEventData;
    static void
    at_run(AtEventData* data,
           const object::objects_type& = object::objects_type());


    // Work around limitations of VC++ 2005.
#define FINALLY_at_run(DefineOrUse)             \
    FINALLY_ ## DefineOrUse                     \
    (at_run,                                    \
     ((bool&, squash))                          \
     ((bool, prev))                             \
     ((bool&, dependencies_log)),               \
     squash = prev; dependencies_log = false)

#define FINALLY_Do(DefineOrUse)                 \
    FINALLY_ ## DefineOrUse                     \
    (Do,                                        \
     ((Stacks&, stacks_))                       \
     ((rObject&, old_tgt)),                     \
     stacks_.this_switch(old_tgt))

#define FINALLY_Local(DefineOrUse)              \
    FINALLY_ ## DefineOrUse                     \
    (Local,                                     \
     ((bool&, squash))                          \
     ((bool, prev)),                            \
     squash = prev)

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

    FINALLY_at_run(DEFINE);
    FINALLY_Do(DEFINE);
    FINALLY_Local(DEFINE);
    FINALLY_Scope(DEFINE);
    FINALLY_Try(DEFINE);

  };

} // namespace runner

# include <runner/interpreter.hxx>

#endif // !RUNNER_INTERPRETER_HH
