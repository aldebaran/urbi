/**
 ** \file runner/runner.hh
 ** \brief Definition of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HH
# define RUNNER_RUNNER_HH

#include <boost/tuple/tuple.hpp>

# include "ast/default-visitor.hh"
# include "object/object.hh"
# include "scheduler/scheduler.hh"
# include "scheduler/job.hh"

namespace runner
{

  /// Ast executor.
  class Runner : public ast::DefaultVisitor,
		 public scheduler::Job
  {
  public:
    /// \name Useful shorthands.
    /// \{
    /// Super class type.
    typedef ast::DefaultVisitor super_type;
    typedef object::rObject rObject;
    typedef object::rLobby rLobby;
    /// \}

    /// \name Ctor & dtor.
    /// \{
    /// Construct a \c Runner in the \a lobby.  The runner needs to
    /// know its \a locals, who is its \a scheduler and will execute 
    /// \a ast.  Memory ownership of \a ast is transferred to the Runner.
    Runner (rLobby lobby, rObject locals,
	    scheduler::Scheduler& scheduler, ast::Ast* ast);

    /// Create a copy of a runner
    Runner (const Runner&);

    /// Destroy a Runner.
    virtual ~Runner ();
    /// \}

    /// \ name Accessors.
    /// \{
  public:
    /// Return the lobby in which this runner has been started.
    const rLobby& lobby_get () const;
    /// \}

    /// Execute the code of function \a func with arguments \a args in the
    /// local runner after installing \a scope as the current context. If
    /// \a scope is 0, a new scope will be created as needed to bind
    /// the function formals in the case of a function written in Urbi.
    rObject apply (rObject scope, const rObject& func,
		   const object::objects_type& args);

  protected:
    /// \name Evaluation.
    /// \{
    /// Evaluate a tree and return the \a current_ that results.
    rObject eval (ast::Ast& e);

    // FIXME: For the time being, if there is no target, it is the
    // Connection object which is used, sort of a Lobby for Io.
    rObject target (ast::Exp* n);

    /// Import from super.
    using super_type::operator();

    VISITOR_VISIT_NODES((16,
			 (
			  And,
			  Call,
			  Float,
			  Function,
			  If,
			  List,
			  Message,
			  Nary,
			  Noop,
			  Pipe,
			  Scope,
			  Stmt,
			  String,
			  Tag,
			  Throw,
			  While
			  )))
    /// \}

    /// Do the actual work.  Implementation of \c Job::run.
    virtual void work ();

  private:
    void raise_error_ (const object::UrbiException& ue);
    void send_message_ (const std::string& text, const std::string& tag);
    rObject apply_urbi (rObject scope, const rObject& func,
			const object::objects_type& args);


  private:
    /// The URBI Lobby used to evaluate.
    /// Wraps an UConnection (ATM).
    rLobby lobby_;

    /// The root of the AST being executed.
    ast::Ast* ast_;

    /// The current value during the evaluation of the AST.
    rObject current_;

    /// The (current set of) local variables, slots of the "locals" object.
    rObject locals_;

    std::list<ast::loc> callStack;
  };

} // namespace runner

# include "runner/runner.hxx"

#endif // !RUNNER_RUNNER_HH
