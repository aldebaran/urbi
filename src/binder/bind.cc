#include "binder/bind.hh"
#include "binder/binder.hh"

namespace binder
{
  void
  bind(ast::Ast& a)
  {
    Binder bind;
    bind(a);
  }
} // namespace binder
