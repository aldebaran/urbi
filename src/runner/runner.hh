/**
 ** \file runner/runner.hh
 ** \brief Definition of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HH
# define RUNNER_RUNNER_HH

# include "ast/default-visitor.hh"

namespace runner
{

  /// Ast pretty-printer.
  class Runner : public ast::DefaultConstVisitor
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Super class type.
    typedef ast::DefaultConstVisitor super_type;

    /// Construct a Runner.
    Runner ();

    /// Destroy a Runner.
    virtual ~Runner ();
    /** \} */

    using super_type::operator();
  };

} // namespace runner

# include "runner/runner.hxx"

#endif // !RUNNER_RUNNER_HH
