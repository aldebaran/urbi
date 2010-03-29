/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <binder/bind.hh>
#include <flower/flow.hh>
#include <rewrite/rewrite.hh>
#include <parser/transform.hh>

#include <ast/nary.hh>

namespace parser
{
  template <typename T>
  libport::intrusive_ptr<T>
  transform(libport::intrusive_ptr<T> ast)
  {
    return binder::bind(rewrite::rewrite(ast::rConstNary(flower::flow(ast))));
  }

#define INST(Type)                              \
  template libport::intrusive_ptr<ast::Type>    \
  transform(libport::intrusive_ptr<ast::Type>); \

  INST(Ast);
  INST(Nary);
} // namespace parser
