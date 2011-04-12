/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file runner/eval/ast.cc
 ** \brief Definition of eval::ast.
 */

#include <libport/config.h>

#include <eval/ast.hh>

#ifndef LIBPORT_COMPILATION_MODE_SPEED
# include <eval/ast.hxx>
#endif

namespace ast
{

#define DEFINE(Class)                                           \
  urbi::object::rObject                                         \
  Class::eval(runner::Job& r) const                             \
  {                                                             \
    urbi::object::rObject res = eval::ast_impl::eval(r, this);  \
    return assert_exp(res);                                     \
  }

  // FIXME: Move to AST_FOR_EACH_NODE.
  DEFINE(And);
  DEFINE(Assign);
  DEFINE(Assignment);
  DEFINE(At);
  DEFINE(Binding);
  DEFINE(Break);
  DEFINE(Call);
  DEFINE(CallMsg);
  DEFINE(Catch);
  DEFINE(Class);
  DEFINE(Continue);
  DEFINE(Declaration);
  DEFINE(Decrementation);
  DEFINE(Dictionary);
  DEFINE(Do);
  DEFINE(Emit);
  DEFINE(Event);
  DEFINE(Finally);
//  DEFINE(Flavored);
  DEFINE(Float);
  DEFINE(Foreach);
  DEFINE(If);
  DEFINE(Implicit);
  DEFINE(Incrementation);
//  DEFINE(LValueArgs);
  DEFINE(List);
  DEFINE(Local);
  DEFINE(LocalAssignment);
  DEFINE(LocalDeclaration);
  DEFINE(Match);
  DEFINE(MetaArgs);
  DEFINE(MetaCall);
  DEFINE(MetaExp);
  DEFINE(MetaId);
  DEFINE(MetaLValue);
  DEFINE(Nary);
  DEFINE(Noop);
  DEFINE(OpAssignment);
  DEFINE(Pipe);
  DEFINE(Property);
//  DEFINE(PropertyAction);
  DEFINE(PropertyWrite);
  DEFINE(Return);
  DEFINE(Routine);
  DEFINE(Scope);
  DEFINE(Stmt);
  DEFINE(String);
  DEFINE(Subscript);
  DEFINE(TaggedStmt);
  DEFINE(This);
  DEFINE(Throw);
  DEFINE(Try);
  DEFINE(Unscope);
  DEFINE(Watch);
  DEFINE(While);
//  DEFINE(Write);
#undef DEFINE
} // namespace ast
