/**
 ** \file runner/runner.hh
 ** \brief Definition of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HH
# define RUNNER_RUNNER_HH

# include <vector>

# include <boost/tuple/tuple.hpp>

# include "ast/default-visitor.hh"
# include "object/object.hh"
# include "scheduler/scheduler.hh"
# include "scheduler/job.hh"

namespace runner
{

  /// Ast executor.
  class Runner : public ast::DefaultConstVisitor,
		 public scheduler::Job,
                 public boost::noncopyable
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
    /// Construct a \c Runner in the \a lobby.  The runner needs to
    /// know its \a locals, who is its \a scheduler and will execute
    /// \a ast.  Memory ownership of \a ast is transferred to the Runner.
    /// The new runner has no parent.
    Runner (rLobby lobby, rObject locals,
	    scheduler::Scheduler& scheduler, const ast::Ast* ast);

    Runner (rLobby lobby, rObject locals,
	    scheduler::Scheduler& scheduler, rObject code);

    /// Create a copy of a runner starting with another ast.
    Runner (const Runner&, const ast::Ast* ast);

    void init(rObject lobby);

    /// Destroy a Runner.
    virtual ~Runner ();
    /// \}

    /// \ name Accessors.
    /// \{
  public:
    /// Return the lobby in which this runner has been started.
    const rLobby& lobby_get () const;
    rLobby lobby_get ();

    /// Return the current locals for this runner.
    const rObject& locals_get () const;
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
    rObject apply (const rObject& func,
		   const libport::Symbol msg,
		   object::objects_type args,
		   rObject call_message = 0);

    /// Use an argument list coming from Urbi.
    rObject apply (const rObject& func, const libport::Symbol msg,
		   const object::rList& args);

    /// Evaluate an expression in the current scope and return its result.
    rObject eval (const ast::Exp&);

    /// Evaluate a tag and create it as well as the intermediate levels
    /// if needed.
    rObject eval_tag (const ast::Exp&);

    /// Make an urbi function from an ast chunk
    rObject
    make_code(const ast::Function& f) const;

  protected:
    /// \name Evaluation.
    /// \{

    std::pair<rObject, rObject>
    target (const ast::Exp* n, const libport::Symbol& name);
    typedef std::pair<bool, const Runner::rObject> locate_type;

    /// Build an evaluated arguments list containing \a tgt and
    /// arguments coming from \a args evaluated in the current context.
    /// If check_void is true, raise an error if any argument is void.
    void push_evaluated_arguments (object::objects_type& args,
				   const ast::exps_type& ue_args,
				   bool check_void);

    /// Build a call message
    rObject build_call_message (const rObject& tgt, const libport::Symbol& msg,
				const object::objects_type& args);

    /// Build a call message
    rObject build_call_message (const rObject& tgt, const libport::Symbol& msg,
				const ast::exps_type& args);

    /// Import from super.
    using super_type::visit;

    CONST_VISITOR_VISIT_NODES((And)
                              (Call)
                              (Do)
                              (Float)
                              (Foreach)
                              (Function)
                              (If)
                              (List)
                              (Message)
                              (Nary)
                              (Noop)
                              (Pipe)
                              (Scope)
                              (Stmt)
                              (String)
                              (Tag)
                              (TaggedStmt)
                              (Throw)
                              (While))

    /// Factor handling of Scope and Do
    void visit (const ast::AbstractScope& e, rObject locals);

    /// \}



    /// Do the actual work.  Implementation of \c Job::run.
    virtual void work ();

  private:
    void show_error_ (const object::UrbiException& ue);
    void propagate_error_ (object::UrbiException& ue, const ast::loc& l);
    void send_message_ (const std::string& tag, const std::string& msg);
    rObject apply_urbi (const rObject& func,
			const libport::Symbol& msg,
			const object::objects_type& args,
			rObject call_message);


  private:
    /// The URBI Lobby used to evaluate.
    /// Wraps an UConnection (ATM).
    rLobby lobby_;

    /// The root of the AST being executed.
    const ast::Ast* ast_;

    /// The urbi Code object to execute
    rObject code_;

    /// The current value during the evaluation of the AST.
    rObject current_;

    /// The (current set of) local variables, slots of the "locals" object.
    rObject locals_;

    /// The call stack.
    typedef std::vector<const ast::Call*> call_stack_type;
    call_stack_type call_stack_;
  };

} // namespace runner

# include "runner/runner.hxx"

#endif // !RUNNER_RUNNER_HH
