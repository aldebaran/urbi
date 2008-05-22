/**
 ** \file binder/bind.hh
 ** \brief Definition of binder::bind().
 */

#ifndef BINDER_BIND_HH
# define BINDER_BIND_HH

namespace ast
{
  class Ast;
}

namespace binder
{

  /// Bind names in \a a.
  void bind(ast::Ast& a);

} // namespace binder

#endif // !BINDER_BINDER_HH
