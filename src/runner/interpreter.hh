/**
 ** \file runner/interpreter.hh
 ** \brief Definition of runner::Interpreter.
 */

#ifndef RUNNER_INTERPRETER_HH
# define RUNNER_INTERPRETER_HH

# include <boost/tuple/tuple.hpp>

# include <libport/compilation.hh>
# include <libport/hash.hh>

# include <ast/fwd.hh>

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
                const libport::Symbol& name);

    Interpreter(const Interpreter&,
                rObject code,
                const libport::Symbol& name,
                const objects_type& args = objects_type());

    /// Create a copy of a runner starting with another ast.
    Interpreter(const Interpreter&,
                ast::rConstAst ast,
                const libport::Symbol& name);

    /// Destroy a Interpreter.
    virtual ~Interpreter();
    /// \}

    /// The entry point: visit \a e.
    ATTRIBUTE_ALWAYS_INLINE object::rObject operator() (const ast::Ast* e);

    /// \ name Accessors.
    /// \{
  public:
    /// \}

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
    virtual rObject apply(const rObject& function,
                          const libport::Symbol msg,
                          const object::objects_type& args,
                          const rObject& call_message = 0);

    rObject apply(const rObject& function,
                  const libport::Symbol msg,
                  const object::objects_type& args,
                  const rObject& call_message,
                  boost::optional<ast::loc> loc);

    virtual rObject apply_call_message(const rObject& function,
                               const libport::Symbol msg,
                               const rObject& call_message);

    rObject apply_call_message(const rObject& function,
                               const libport::Symbol msg,
                               const rObject& call_message,
                               boost::optional<ast::loc> loc);

    /// Apply a function with the arguments as ast chunks.
    rObject apply_ast(const rObject& tgt,
                      const libport::Symbol& msg,
                      const ast::exps_type* args,
                      boost::optional<ast::loc> loc);
    rObject apply_ast(const rObject& tgt,
                      const rObject& f,
                      const libport::Symbol& msg,
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
    void push_evaluated_arguments (object::objects_type& args,
				   const ast::exps_type& ue_args);

    /// Build a call message
    virtual rObject
    build_call_message(const rObject& code,
		       const libport::Symbol& msg,
                       const object::objects_type& args);

    /// Build a call message
    rObject build_call_message(const rObject& tgt,
			       const rObject& code,
			       const libport::Symbol& msg,
                               const ast::exps_type& args);

  public:

#define VISIT(Macro, Data, Node)                                        \
    LIBPORT_SPEED_ALWAYS_INLINE object::rObject visit(const ast::Node* n);

    AST_FOR_EACH_NODE(VISIT);

#undef VISIT

    /// \}


    /// Do the actual work.  Implementation of \c Job::run.
    virtual void work ();

    /// Signal a scheduling error exception.
    ///
    /// \param msg The explanation of the scheduling error.
    virtual void scheduling_error(const std::string& msg);

    virtual void show_backtrace(const std::string& chan);
    virtual backtrace_type backtrace_get() const;
    object::call_stack_type call_stack_get() const;

    /// Throw an excpetion.
    /**
     * \param exn Value to throw
     * \param skip_last Skip last call when reporting the backtrace
     */
    virtual void raise(rObject exn, bool skip_last = false)
      __attribute__ ((noreturn));

    virtual libport::Symbol innermost_call_get() const;

  protected:
    void show_exception_ (object::UrbiException& ue);

  private:
    void init();
    /// Reset result_, set the location and call stack of ue.
    rObject apply_urbi (const rCode& function,
			const libport::Symbol& msg,
			const object::objects_type& args,
			const rObject& call_message);

  private:
    /// Factor handling of Function and Closure
    object::rObject visit(const ast::Routine* e, bool closure);

    /// The root of the AST being executed.
    ast::rConstAst ast_;

    /// The urbi Code object to execute
    rObject code_;
    /// Its arguments
    objects_type args_;

    /// The current value during the evaluation of the AST.
    rObject result_;

    /// The call stack.
    typedef object::call_stack_type call_stack_type;
    typedef object::call_type call_type;
    call_stack_type call_stack_;
    void show_backtrace(const call_stack_type& bt,
                        const std::string& chan);

    /// The local variable stacks
    Stacks stacks_;

    const ast::Ast* innermost_node_;

    /// The current exception when executing a "catch" block.
    rObject current_exception_;
  };

} // namespace runner

# include <runner/interpreter.hxx>

#endif // !RUNNER_INTERPRETER_HH
