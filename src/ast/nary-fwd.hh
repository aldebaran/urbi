/**
 ** \file ast/nary-fwd.hh
 ** \brief Forward declarations of some AST classes
 */

#ifndef AST_NARY_FWD_HH
# define AST_NARY_FWD_HH

# include <libport/shared-ptr.hh>

namespace ast
{
  class Ast;
  typedef libport::shared_ptr<Ast> rAst;
  typedef libport::shared_ptr<const Ast> rConstAst;

  class Nary;
  typedef libport::shared_ptr<Nary> rNary;
  typedef libport::shared_ptr<const Nary> rConstNary;
}

#endif AST_NARY_FWD_HH
