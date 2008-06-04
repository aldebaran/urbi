#include <binder/bind.hh>
#include <binder/binder.hh>

namespace binder
{
  ast::rAst
  bind(ast::rConstAst a)
  {
    Binder bind;
    bind(a);
    return bind.result_get();
  }
} // namespace binder
