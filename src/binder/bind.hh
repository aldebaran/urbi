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

  /// How to bind. Essentialy, in normal mode, unknown variables
  /// are searched in self, while they're search in the context in
  /// context mode.
  enum bind_type
  {
    bind_normal,
    bind_context,
  };

  /// Bind names in \a a.
  void bind(ast::Ast& a, bind_type mode = bind_normal);

} // namespace binder

#endif // !BINDER_BINDER_HH
