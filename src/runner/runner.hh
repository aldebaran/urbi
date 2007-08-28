/**
 ** \file runner/runner.hh
 ** \brief Definition of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HH
# define RUNNER_RUNNER_HH

# include "ast/default-visitor.hh"
# include "object/object.hh"
# include "runner/coroutine.hh"
# include "runner/scheduler.hh"

namespace runner
{

  /// Ast executor.
  class Runner : public ast::DefaultVisitor,
		 public Coroutine
  {
  public:
    /// \name Useful shorthands.
    /// \{
    /// Super class type.
    typedef ast::DefaultVisitor super_type;
    typedef object::rObject rObject;
    typedef object::rContext rContext;
    /// \}

    /// \name Ctor & dtor.
    /// \{
    /// Construct a \c Runner in the context \a ctx.  The runner needs to
    /// know who is its \a scheduler and will execute \a ast.  Memory
    /// ownership of \a ast is transferred to the Runner.
    Runner (rContext ctx, Scheduler& scheduler, ast::Ast* ast);

    /// Destroy a Runner.
    virtual ~Runner ();
    /// \}

  protected:
    /// \name Evaluation.
    /// \{
    /// Evaluate a tree and return the \a current_ that results.
    rObject eval (ast::Ast& e);

    /** Send a @a result to the context.  The @a result is not sent if it
     *  doesn't point to anything.  */
    void emit_result (rObject result);

    // FIXME: For the time being, if there is no target, it is the
    // Connection object which is used, sort of a Lobby for Io.
    rObject target (ast::Exp* n);

    /// Import from super.
    using super_type::operator();

    virtual void operator() (ast::AssignExp& e);
    virtual void operator() (ast::AndExp& e);
    virtual void operator() (ast::CallExp& e);
    virtual void operator() (ast::FloatExp& e);
    virtual void operator() (ast::Function& e);
    virtual void operator() (ast::ListExp& e);
    virtual void operator() (ast::NegOpExp& e);
    virtual void operator() (ast::Noop& e);
    virtual void operator() (ast::PipeExp& e);
    virtual void operator() (ast::SemicolonExp& e);
    virtual void operator() (ast::StringExp& e);
    /// \}

    /// Do the actual work.  Implementation of \c Job::run.
    virtual void work ();
    /// Re-implementation of \c Job::stop.
    virtual void stop ();
    /// Re-implementation of \c Coroutine::finished.
    virtual void finished (Coroutine& coro);

  private:
    /// The URBI Context used to evaluate.
    /// Wraps an UConnection (ATM).
    rContext context_;

    /// The current value during the evaluation of the AST.
    rObject current_;

    /// The root of the AST being executed.
    ast::Ast* ast_;

    /// Whether or not we started to execute anything.
    bool started_;
  };

} // namespace runner

# include "runner/runner.hxx"

#endif // !RUNNER_RUNNER_HH
