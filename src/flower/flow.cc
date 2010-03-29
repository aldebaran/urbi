/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <flower/flow.hh>
#include <flower/flower.hh>
#include <kernel/server-timer.hh>

namespace flower
{
  ast::rNary
  flow(ast::rConstAst a)
  {
    TIMER_PUSH("flow");
    Flower flow;
    ast::rExp res = ast::analyze(flow, a);
    TIMER_POP("flow");
    return res ? res.unchecked_cast<ast::Nary>() : ast::rNary();
  }

} // namespace flower
