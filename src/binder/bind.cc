#include "binder/bind.hh"
#include "binder/binder.hh"

namespace binder
{
  void
  bind(ast::Ast& a, bind_type mode)
  {
    Binder bind(mode);
    bind(a);
  }

} // namespace binder
