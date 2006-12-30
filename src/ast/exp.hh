//<<-
// Generated, do not edit by hand.
//->>
/**
 ** \file ast/exp.hh
 ** \brief Declaration of ast::Exp.
 */

#ifndef AST_EXP_HH
# define AST_EXP_HH

# include "ast/ast.hh"

namespace ast
{

  /// Exp.
  class Exp: public Ast
  {
    /** \name Ctor & dtor.
     ** \{ */
     public:
    /// Construct an Exp node.
    Exp (const loc& location);
    /// Destroy an Exp node.
    virtual ~Exp ();
    /** \} */
  };

} // namespace ast

# include "ast/exp.hxx"

#endif // !AST_EXP_HH
