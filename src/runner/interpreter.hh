/**
 ** \file runner/interpreter.hh
 ** \brief Definition of runner::Interpreter.
 */

#ifndef RUNNER_INTERPRETER_HH
# define RUNNER_INTERPRETER_HH

# include <vector>

# include <boost/tuple/tuple.hpp>

# include <ast/default-visitor.hh>
# include <ast/fwd.hh>
# include <object/object.hh>
# include <scheduler/scheduler.hh>
# include <scheduler/job.hh>

# include <runner/runner.hh>

namespace runner
{

  /// Ast executor.
  class Interpreter : public ast::DefaultConstVisitor,
		      public runner::Runner
  {
  public:
    /// \name Useful shorthands.
    /// \{
    /// Super class type.
    typedef ast::DefaultConstVisitor super_type;
    typedef object::rObject rObject;
    typedef object::rLobby rLobby;
    /// \}

    /// \name Ctor & dtor.
    /// \{
    /// Construct a \c Interpreter in the \a lobby.  The runner needs
    /// to know its \a scheduler and will execute \a ast.  Memory
    /// ownership of \a ast is transferred to the Interpreter.  The
    /// new runner has no parent.
    Interpreter (rLobby lobby,
		 scheduler::Scheduler& scheduler,
		 ast::rConstAst ast,
		 const libport::Symbol& name = SYMBOL());

    Interpreter (const Interpreter&,
		 rObject code,
		 const libport::Symbol& name = SYMBOL());

    /// Create a copy of a runner starting with another ast.
    Interpreter (const Interpreter&,
		 ast::rConstAst ast,
		 const libport::Symbol& name = SYMBOL());

    /// Destroy a Interpreter.
    virtual ~Interpreter ();
    /// \}

    /// The entry point: visit \a e.
    virtual void operator() (ast::rConstAst e);

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
    /// One cannot have both a call message and args.
    virtual rObject apply (const rObject& func,
			   const libport::Symbol msg,
			   object::objects_type args,
			   rObject call_message = 0);

    /// Helper to apply a function with the arguments as ast chunks
    rObject apply (rObject tgt, const libport::Symbol& msg,
                   const ast::exps_type* args);

    /// Use an argument list coming from Urbi.
    virtual rObject apply (const rObject& func, const libport::Symbol msg,
			   const object::rList& args);


    /// Evaluate an expression in the current scope and return its result.
    rObject eval (ast::rConstAst);

    /// Evaluate a tag and create it as well as the intermediate levels
    /// if needed.
    rObject eval_tag (ast::rConstExp);

    /// Make an urbi function from an ast chunk
    rObject make_code(ast::rConstCode f) const;

    /// Return the current scope_tag, after creating it if needed.
    scheduler::rTag scope_tag();

  protected:
    /// \name Evaluation.
    /// \{
    typedef std::pair<bool, const Interpreter::rObject> locate_type;

    /// Build an evaluated arguments list containing \a tgt and
    /// arguments coming from \a args evaluated in the current context.
    /// If check_void is true, raise an error if any argument is void.
    void push_evaluated_arguments (object::objects_type& args,
				   const ast::exps_type& ue_args,
				   bool check_void);

    /// Build a call message
    virtual rObject build_call_message
      (const rObject& tgt, const libport::Symbol& msg,
       const object::objects_type& args);

    /// Build a call message
    rObject build_call_message (const rObject& tgt, const libport::Symbol& msg,
				const ast::exps_type& args);

    /// Import from super.
    using super_type::visit;

    CONST_VISITOR_VISIT_NODES((And)
                              (Assignment)
                              (Call)
                              (CallMsg)
                              (Closure)
                              (Do)
                              (Declaration)
                              (Float)
                              (Foreach)
                              (Function)
                              (If)
                              (List)
                              (Local)
                              (Message)
                              (Nary)
                              (Noop)
                              (Pipe)
                              (Scope)
                              (Stmt)
                              (String)
                              (Tag)
                              (TaggedStmt)
                              (This)
                              (Throw)
                              (While))

    /// Factor handling of Scope and Do
    void visit (ast::rConstAbstractScope e, rObject locals);
    /// \}


    /// Do the actual work.  Implementation of \c Job::run.
    virtual void work ();


    virtual void show_backtrace(const std::string& chan);
    virtual backtrace_type backtrace_get() const;

  protected:
    void show_error_ (const object::UrbiException& ue);

  private:
    void init();
    void propagate_error_ (object::UrbiException& ue, const ast::loc& l);
    rObject apply_urbi (const rObject& func,
			const libport::Symbol& msg,
			const object::objects_type& args,
			rObject call_message);
    void cleanup_scope_tag();

  private:
    /// The root of the AST being executed.
    ast::rConstAst ast_;

    /// The urbi Code object to execute
    rObject code_;

    /// The current value during the evaluation of the AST.
    rObject current_;

    /// The (current set of) local variables, slots of the "locals" object.
    rObject locals_;

    /// The scope tags stack.
    std::vector<scheduler::rTag> scope_tags_;

    /// The call stack.
    typedef object::UrbiException::call_stack_type call_stack_type;
    call_stack_type call_stack_;
    void show_backtrace(const call_stack_type& bt,
                        const std::string& chan);

    /// Retreive the n-frames-above context
    rObject context(unsigned n);
  };

} // namespace runner

# include <runner/interpreter.hxx>

#endif // !RUNNER_INTERPRETER_HH
