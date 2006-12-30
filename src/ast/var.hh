//<<-
// Generated, do not edit by hand.
//->>
/**
 ** \file ast/var.hh
 ** \brief Declaration of ast::Var.
 */

#ifndef AST_VAR_HH
# define AST_VAR_HH

# include "ast/exp.hh"

namespace ast
{

  /// Var.
  class Var: public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
     public:
    /// Construct a Var node.
    Var (const loc& location);
    /// Destroy a Var node.
    virtual ~Var ();
    /** \} */

    /** \name Visitors entry point.
     ** \{ */
  public:
    /// Accept a const visitor \a v.
    virtual void accept (ConstVisitor& v) const;
    /// Accept a non-const visitor \a v.
    virtual void accept (Visitor& v);
    /** \} */
  };

} // namespace ast

# include "ast/var.hxx"

#endif // !AST_VAR_HH
