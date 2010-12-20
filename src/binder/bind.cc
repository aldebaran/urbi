/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <ast/nary.hh>
#include <binder/bind.hh>
#include <binder/binder.hh>

#include <kernel/server-timer.hh>

#include <urbi/object/object.hh>

namespace binder
{
  template <typename T>
  libport::intrusive_ptr<T>
  bind(libport::intrusive_ptr<T> a)
  {
    TIMER_PUSH("bind");
    Binder bind;
    ast::rAst res = ast::analyze(bind, a);
    TIMER_POP("bind");
    return res ? res.unchecked_cast<T>() : libport::intrusive_ptr<T>();
  }

#define INSTANTIATE(Type)                               \
 template libport::intrusive_ptr<Type> bind<Type>(libport::intrusive_ptr<Type>)
  INSTANTIATE(ast::Ast);
  INSTANTIATE(const ast::Ast);
  INSTANTIATE(ast::Exp);
  INSTANTIATE(const ast::Exp);
  INSTANTIATE(ast::Nary);
  INSTANTIATE(const ast::Nary);
#undef INSTANTIATE

} // namespace binder
