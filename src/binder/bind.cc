#include "binder/bind.hh"
#include "binder/binder.hh"

namespace binder
{
  void
  bind(ast::rAst a)
  {
    Binder bind;
    bind(a);
  }
} // namespace binder
