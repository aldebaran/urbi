#include <flower/flow.hh>
#include <flower/flower.hh>
#include <kernel/server-timer.hh>

namespace flower
{

  ast::rNary
  flow(ast::rConstNary a)
  {
    TIMER_PUSH("flow");
    Flower flow;
    ast::rExp res = ast::analyze(flow, a);
    TIMER_POP("flow");
    return res ? res.unchecked_cast<ast::Nary>() : ast::rNary();
  }


} // namespace flower
