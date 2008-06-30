/**
 ** \file binder/bind.hh
 ** \brief Definition of binder::bind().
 */

#ifndef BINDER_BIND_HH
# define BINDER_BIND_HH

# include <ast/nary-fwd.hh>

namespace binder
{

  /// Bind names in \a a.
  ast::rNary bind(ast::rConstNary a);

} // namespace binder

#endif // !BINDER_BINDER_HH
