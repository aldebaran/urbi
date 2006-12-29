//<<-
// Generated, do not edit by hand.
//->>
/**
 ** \file ast/ast.hxx
 ** \brief Inline methods of ast::Ast.
 */

#ifndef AST_AST_HXX
# define AST_AST_HXX

# include "ast/ast.hh"

namespace ast
{

  inline const location&
  Ast::location_get () const
  {
    return location_;
  }
  inline void
  Ast::location_set (const location& location)
  {
    location_ = location;
  }

  inline const Ast&
  Ast::up_get () const
  {
    return *up_;
  }
  inline void
  Ast::up_set (Ast* up)
  {
    up_ = up;
  }


} // namespace ast

#endif // !AST_AST_HXX
