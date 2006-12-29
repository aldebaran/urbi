//<<-
// Generated, do not edit by hand.
//->>
/**
 ** \file ast/ast.cc
 ** \brief Implementation of ast::Ast.
 */

#include "ast/visitor.hh"
#include "ast/ast.hh"

namespace ast
{

  Ast::Ast (const location& location):
    location_ (location),
    up_ (0)
  { }

  Ast::~Ast ()
  {
    delete up_;
  }


} // namespace ast

