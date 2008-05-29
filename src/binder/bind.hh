/**
 ** \file binder/bind.hh
 ** \brief Definition of binder::bind().
 */

#ifndef BINDER_BIND_HH
# define BINDER_BIND_HH

# include <libport/shared-ptr.hh>

namespace ast
{
  class Ast;
  typedef libport::shared_ptr<Ast> rAst;
  typedef libport::shared_ptr<const Ast> rConstAst;
}

namespace binder
{

  /// Bind names in \a a.
  ast::rAst bind(ast::rConstAst a);

} // namespace binder

#endif // !BINDER_BINDER_HH
