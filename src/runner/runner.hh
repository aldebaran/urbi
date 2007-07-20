/**
 ** \file runner/runner.hh
 ** \brief Definition of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HH
# define RUNNER_RUNNER_HH

# include "ast/default-visitor.hh"
# include "object/object.hh"

namespace runner
{

  /// Ast pretty-printer.
  class Runner : public ast::DefaultConstVisitor
  {
  public:
    /// \name Useful shorthands.
    /// \{
    /// Super class type.
    typedef ast::DefaultConstVisitor super_type;
    ///
    typedef object::rObject rObject;
    /// \}

    /// \name Ctor & dtor.
    /// \{
    /// Construct a Runner.
    Runner ();

    /// Destroy a Runner.
    virtual ~Runner ();
    /// \}

    /// \name Evaluation.
    /// \{
    /// Evaluate a tree and return the \a current_ that results.
    rObject eval (const ast::Ast& e);
    rObject result ();

    // FIXME: For the time being, if there is no target, it is the
    // Connection object which is used, sort of a Lobby for IO.
    rObject target (ast::Exp* n);

    /// Import from super.
    using super_type::operator();

    virtual void operator() (const ast::AssignExp& e);
    virtual void operator() (const ast::CallExp& e);
    virtual void operator() (const ast::FloatExp& e);
    virtual void operator() (const ast::Function& e);
    virtual void operator() (const ast::SemicolonExp& e);
    /// \}

  private:
    /// The current value during the evaluation of the ast.
    rObject current_;
  };

} // namespace runner

# include "runner/runner.hxx"

#endif // !RUNNER_RUNNER_HH
