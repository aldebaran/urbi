/**
 ** \file ast/nary-fwd.hh
 ** \brief Forward declarations of some AST classes
 */

#ifndef AST_NARY_FWD_HH
# define AST_NARY_FWD_HH

# include <libport/intrusive-ptr.hh>

namespace ast
{
  class Ast;
  typedef libport::intrusive_ptr<Ast> rAst;
  typedef libport::intrusive_ptr<const Ast> rConstAst;

  class Nary;
  typedef libport::intrusive_ptr<Nary> rNary;
  typedef libport::intrusive_ptr<const Nary> rConstNary;
}

#endif // AST_NARY_FWD_HH
