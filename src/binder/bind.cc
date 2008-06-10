#include <binder/bind.hh>
#include <binder/binder.hh>

#include <server-timer.hh>

namespace binder
{
  ast::rAst
  bind(ast::rConstAst a)
  {
    TIMER_PUSH("bind");
    Binder bind;
    bind(a);
    ast::rAst ast = bind.result_get();
    TIMER_POP("bind");
    return ast;
  }
} // namespace binder
