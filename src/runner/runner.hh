/**
 ** \file runner/runner.hh
 ** \brief Definition of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HH
# define RUNNER_RUNNER_HH

# include <boost/tuple/tuple.hpp>

# include "ast/default-visitor.hh"
# include "object/object.hh"
# include "runner/tag.hh"
# include "scheduler/scheduler.hh"
# include "scheduler/job.hh"

namespace runner
{

  /// Ast executor.
  class Runner : public ast::DefaultConstVisitor,
		 public scheduler::Job,
		 public Tag
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

    /// Create a copy of a runner.
    Runner (const Runner&);

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
    rObject apply (const rObject& func,
		   const object::objects_type& args,
		   const rObject call_message = 0);

    /// Use an argument list coming from Urbi.
    rObject apply (const rObject& func, const object::rList& args);

    /// Eval a tree in a given local scope
    rObject eval_in_scope (rObject scope, const ast::Exp& e);

    /// Return the result of an evaluation. The runner must be terminated.
    const rObject& current_get () const;

    /// Return a shared pointer on myself. The runner must not be terminated.
    scheduler::rJob myself_get () const;
  protected:
    /// \name Evaluation.
    /// \{
    /// Evaluate a tree and return the \a current_ that results.
    rObject eval (const ast::Ast& e);

    // FIXME: For the time being, if there is no target, it is the
    // Connection object which is used, sort of a Lobby for Io.
    rObject target (const ast::Exp* n);

    /// Build an evaluated arguments list containing \a tgt and
    /// arguments coming from \a args evaluated in the current context.
    void push_evaluated_arguments (object::objects_type& args,
				   const ast::exps_type& ue_args);

    /// Build a call message
    rObject build_call_message (const rObject& tgt, const libport::Symbol& msg,
				const ast::exps_type& args) const;

    /// Import from super.
    using super_type::operator();

    CONST_VISITOR_VISIT_NODES((16,
			       (
				 And,
				 Call,
				 Foreach,
				 Function,
				 If,
				 List,
				 Message,
				 Nary,
				 Noop,
				 Object,
				 Pipe,
				 Scope,
				 Stmt,
				 Tag,
				 Throw,
				 While
				 )))
    /// \}

    /// Code to run the cleanup code
    void run_at_exit (object::rObject& scope);

    /// Code to execute when terminating a runner.
    virtual void terminate ();

    /// Do the actual work.  Implementation of \c Job::run.
    virtual void work ();

    /// Scheduling operations
    virtual void act (operation_type);

  private:
    void show_error_ (object::UrbiException& ue, const ast::loc& l);
    void send_message_ (const std::string& tag, const std::string& msg);
    rObject apply_urbi (const rObject& func,
			const object::objects_type& args,
			const rObject call_message);


  private:
    /// The URBI Lobby used to evaluate.
    /// Wraps an UConnection (ATM).
    rLobby lobby_;

    /// The root of the AST being executed.
    const ast::Ast* ast_;

    /// The current value during the evaluation of the AST.
    rObject current_;

    /// The (current set of) local variables, slots of the "locals" object.
    rObject locals_;

    /// Myself as long as I have not terminated, 0 otherwise
    scheduler::rJob myself_;

    /// The call stack.
    typedef std::list<const ast::Call*> call_stack_type;
    call_stack_type call_stack_;
  };

} // namespace runner

# include "runner/runner.hxx"

#endif // !RUNNER_RUNNER_HH
