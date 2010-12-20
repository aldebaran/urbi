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
#include <urbi/object/object.hh>

namespace flower
{
  template <typename T>
  libport::intrusive_ptr<T>
  flow(libport::intrusive_ptr<const T> a)
  {
    TIMER_PUSH("flow");
    Flower flow;
    ast::rExp res = ast::analyze(flow, a);
    TIMER_POP("flow");
    return res.unchecked_cast<T>();
  }

#define INST(Type)                                      \
  template libport::intrusive_ptr<ast::Type>            \
  flow(libport::intrusive_ptr<const ast::Type>)

  INST(Ast);
  INST(Exp);
  INST(Nary);
} // namespace flower
