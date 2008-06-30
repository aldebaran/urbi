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
    if (getenv("BIND"))
      LIBPORT_ECHO("Binding: " << *a);
    bind(a);
    ast::rAst ast = bind.result_get();
    TIMER_POP("bind");
    return ast.unsafe_cast<ast::Nary>();
  }

} // namespace binder
