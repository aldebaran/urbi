/*
 * Copyright (C) 2006-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// Generated, do not edit by hand.

/**
 ** \file ast/transformer.cc
 ** \brief Implementation of ast::Transformer.
 */
#include <ast/transformer.hh>

namespace ast
{
  void
  Transformer::visit(And* node)
  {
    transform_collection(node->children_get());
    result_ = node;
  }

  void
  Transformer::visit(Assign* node)
  {
    transform(node->what_get());
    transform(node->value_get());
    result_ = node;
  }

  void
  Transformer::visit(Assignment* node)
  {
    transform(node->what_get());
    transform(node->value_get());
    result_ = node;
  }

  void
  Transformer::visit(Ast* node)
  {
    result_ = node;
  }

  void
  Transformer::visit(At* node)
  {
    transform(node->cond_get());
    transform(node->body_get());
    transform(node->onleave_get());
    transform(node->duration_get());
    result_ = node;
  }

  void
  Transformer::visit(Binding* node)
  {
    transform(node->what_get());
    result_ = node;
  }

  void
  Transformer::visit(Break* node)
  {
    result_ = node;
  }

  void
  Transformer::visit(Call* node)
  {
    transform_collection(node->arguments_get());
    transform(node->target_get());
    result_ = node;
  }

  void
  Transformer::visit(CallMsg* node)
  {
    result_ = node;
  }

  void
  Transformer::visit(Catch* node)
  {
    transform(node->match_get());
    transform(node->body_get());
    result_ = node;
  }

  void
  Transformer::visit(Class* node)
  {
    transform(node->what_get());
    transform_collection(node->protos_get());
    transform(node->content_get());
    result_ = node;
  }

  void
  Transformer::visit(Composite* node)
  {
    transform_collection(node->children_get());
    result_ = node;
  }

  void
  Transformer::visit(Continue* node)
  {
    result_ = node;
  }

  void
  Transformer::visit(Declaration* node)
  {
    transform(node->what_get());
    transform(node->value_get());
    result_ = node;
  }

  void
  Transformer::visit(Decrementation* node)
  {
    transform(node->exp_get());
    result_ = node;
  }

  void
  Transformer::visit(Dictionary* node)
  {
    transform_collection(node->value_get());
    result_ = node;
  }

  void
  Transformer::visit(Do* node)
  {
    transform(node->body_get());
    transform(node->target_get());
    result_ = node;
  }

  void
  Transformer::visit(Emit* node)
  {
    transform(node->event_get());
    transform_collection(node->arguments_get());
    transform(node->duration_get());
    result_ = node;
  }

  void
  Transformer::visit(Event* node)
  {
    transform(node->exp_get());
    result_ = node;
  }

  void
  Transformer::visit(Exp* node)
  {
    result_ = node;
  }

  void
  Transformer::visit(Finally* node)
  {
    transform(node->body_get());
    transform(node->finally_get());
    result_ = node;
  }

  void
  Transformer::visit(Flavored* node)
  {
    result_ = node;
  }

  void
  Transformer::visit(Float* node)
  {
    result_ = node;
  }

  void
  Transformer::visit(Foreach* node)
  {
    transform(node->index_get());
    transform(node->list_get());
    transform(node->body_get());
    result_ = node;
  }

  void
  Transformer::visit(If* node)
  {
    transform(node->test_get());
    transform(node->thenclause_get());
    transform(node->elseclause_get());
    result_ = node;
  }

  void
  Transformer::visit(Implicit* node)
  {
    result_ = node;
  }

  void
  Transformer::visit(Incrementation* node)
  {
    transform(node->exp_get());
    result_ = node;
  }

  void
  Transformer::visit(LValue* node)
  {
    result_ = node;
  }

  void
  Transformer::visit(LValueArgs* node)
  {
    transform_collection(node->arguments_get());
    result_ = node;
  }

  void
  Transformer::visit(List* node)
  {
    transform_collection(node->value_get());
    result_ = node;
  }

  void
  Transformer::visit(Local* node)
  {
    transform_collection(node->arguments_get());
    transform(node->declaration_get());
    result_ = node;
  }

  void
  Transformer::visit(LocalAssignment* node)
  {
    transform(node->value_get());
    transform(node->declaration_get());
    result_ = node;
  }

  void
  Transformer::visit(LocalDeclaration* node)
  {
    transform(node->value_get());
    transform(node->type_get());
    result_ = node;
  }

  void
  Transformer::visit(LocalWrite* node)
  {
    transform(node->value_get());
    result_ = node;
  }

  void
  Transformer::visit(Match* node)
  {
    transform(node->pattern_get());
    transform(node->guard_get());
    transform(node->bindings_get());
    result_ = node;
  }

  void
  Transformer::visit(MetaArgs* node)
  {
    transform(node->lvalue_get());
    result_ = node;
  }

  void
  Transformer::visit(MetaCall* node)
  {
    transform_collection(node->arguments_get());
    transform(node->target_get());
    result_ = node;
  }

  void
  Transformer::visit(MetaExp* node)
  {
    result_ = node;
  }

  void
  Transformer::visit(MetaId* node)
  {
    transform_collection(node->arguments_get());
    result_ = node;
  }

  void
  Transformer::visit(MetaLValue* node)
  {
    transform_collection(node->arguments_get());
    result_ = node;
  }

  void
  Transformer::visit(Nary* node)
  {
    transform_collection(node->children_get());
    result_ = node;
  }

  void
  Transformer::visit(Noop* node)
  {
    transform(node->body_get());
    result_ = node;
  }

  void
  Transformer::visit(OpAssignment* node)
  {
    transform(node->what_get());
    transform(node->value_get());
    result_ = node;
  }

  void
  Transformer::visit(Pipe* node)
  {
    transform_collection(node->children_get());
    result_ = node;
  }

  void
  Transformer::visit(Property* node)
  {
    transform(node->owner_get());
    result_ = node;
  }

  void
  Transformer::visit(PropertyAction* node)
  {
    transform(node->owner_get());
    result_ = node;
  }

  void
  Transformer::visit(PropertyWrite* node)
  {
    transform(node->owner_get());
    transform(node->value_get());
    result_ = node;
  }

  void
  Transformer::visit(Return* node)
  {
    transform(node->value_get());
    result_ = node;
  }

  void
  Transformer::visit(Routine* node)
  {
    transform_collection(node->formals_get());
    transform(node->body_get());
    transform_collection(node->local_variables_get());
    transform_collection(node->captured_variables_get());
    result_ = node;
  }

  void
  Transformer::visit(Scope* node)
  {
    transform(node->body_get());
    result_ = node;
  }

  void
  Transformer::visit(Stmt* node)
  {
    transform(node->expression_get());
    result_ = node;
  }

  void
  Transformer::visit(String* node)
  {
    result_ = node;
  }

  void
  Transformer::visit(Subscript* node)
  {
    transform_collection(node->arguments_get());
    transform(node->target_get());
    result_ = node;
  }

  void
  Transformer::visit(TaggedStmt* node)
  {
    transform(node->tag_get());
    transform(node->exp_get());
    result_ = node;
  }

  void
  Transformer::visit(This* node)
  {
    result_ = node;
  }

  void
  Transformer::visit(Throw* node)
  {
    transform(node->value_get());
    result_ = node;
  }

  void
  Transformer::visit(Try* node)
  {
    transform(node->body_get());
    transform_collection(node->handlers_get());
    transform(node->elseclause_get());
    result_ = node;
  }

  void
  Transformer::visit(Unary* node)
  {
    transform(node->exp_get());
    result_ = node;
  }

  void
  Transformer::visit(Unscope* node)
  {
    result_ = node;
  }

  void
  Transformer::visit(Watch* node)
  {
    transform(node->exp_get());
    result_ = node;
  }

  void
  Transformer::visit(While* node)
  {
    transform(node->test_get());
    transform(node->body_get());
    result_ = node;
  }

  void
  Transformer::visit(Write* node)
  {
    transform(node->what_get());
    transform(node->value_get());
    result_ = node;
  }


}

