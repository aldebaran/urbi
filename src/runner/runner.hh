/**
 ** \file runner/runner.hh
 ** \brief Definition of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HH
# define RUNNER_RUNNER_HH

# include "kernel/fwd.hh"  // UConnection

# include "ast/default-visitor.hh"
# include "object/object.hh"
# include "runner/job.hh"
# include "runner/scheduler.hh"

namespace runner
{

  /// Ast executor.
  class Runner : public ast::DefaultConstVisitor, public Job
  {
  public:
    /// \name Useful shorthands.
    /// \{
    /// Super class type.
    typedef ast::DefaultConstVisitor super_type;
    typedef object::rObject rObject;
    typedef object::rContext rContext;
    /// \}

    /// \name Ctor & dtor.
    /// \{
    /// Construct a Runner.
    Runner (rContext ctx, Scheduler& scheduler);

    /// Destroy a Runner.
    virtual ~Runner ();
    /// \}

    /// \name Evaluation.
    /// \{
    /// Evaluate a tree and return the \a current_ that results.
    rObject eval (const ast::Ast& e);
    rObject result ();

    // FIXME: For the time being, if there is no target, it is the
    // Connection object which is used, sort of a Lobby for Io.
    rObject target (ast::Exp* n);

    /// Import from super.
    using super_type::operator();

    virtual void operator() (const ast::AssignExp& e);
    virtual void operator() (const ast::AndExp& e);
    virtual void operator() (const ast::CallExp& e);
    virtual void operator() (const ast::FloatExp& e);
    virtual void operator() (const ast::Function& e);
    virtual void operator() (const ast::NegOpExp& e);
    virtual void operator() (const ast::SemicolonExp& e);
    virtual void operator() (const ast::StringExp& e);
    virtual void operator() (const ast::ListExp& e);
    /// \}

  protected:
    /// Do the actual work.  Implementation of \c Job::run.
    virtual void work ();

  private:
    /// The URBI Context used to evaluate.
    /// Wraps an UConnection (ATM).
    rContext context_;

    /// The current value during the evaluation of the AST.
    rObject current_;
  };

} // namespace runner

# include "runner/runner.hxx"

#endif // !RUNNER_RUNNER_HH
