#include <ast/nary.hh>
#include <binder/bind.hh>
#include <binder/binder.hh>

#include <kernel/server-timer.hh>

namespace binder
{
  ast::rNary
  bind(ast::rConstNary a)
  {
    if (getenv("BIND"))
      LIBPORT_ECHO("Binding: " << *a);

    TIMER_PUSH("bind");
    Binder bind;
    bind(a);
    TIMER_POP("bind");

    ast::rNary res;
    if (bind.errors_get().empty())
      res = bind.result_get().unsafe_cast<ast::Nary>();
    else
    {
      res = new ast::Nary();
      bind.errors_get().process_errors(*res);
    }
    return res;
  }

} // namespace binder
