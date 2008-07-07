#include <ast/nary.hh>
#include <binder/bind.hh>
#include <binder/binder.hh>

#include <kernel/server-timer.hh>

namespace binder
{
  ast::rNary
  bind(ast::rConstNary a)
  {
    TIMER_PUSH("bind");
    Binder bind;
    ast::rNary res = ast::analyze(bind, a);
    TIMER_POP("bind");
    return res;
  }

} // namespace binder
