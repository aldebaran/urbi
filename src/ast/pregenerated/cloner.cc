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
 ** \file ast/cloner.cc
 ** \brief Implementation of ast::Cloner.
 */
#include "kernel/config.h"
#include "ast/all.hh"
#include "ast/cloner.hh"
// This file contains only protected functions, not loading it
// from cloner.hh saves cycles.
#include "ast/cloner.hxx"
#include "libport/symbol.hh"

namespace ast
{
  using namespace ast;

  Cloner::Cloner (bool map)
   : use_map_(map)
  {}

  Cloner::~Cloner ()
  {}

  rAst
  Cloner::result_get ()
  {
    return result_;
  }

  void Cloner::location_set(const ast::loc& loc)
  {
    loc_ = loc;
  }

  void Cloner::operator() (const Ast* e)
  {
    if (!e)
      return;

    if (use_map_ && libport::has(map_, (long)e))
    {
      result_ = map_[(long)e];
      return;
    }
    // We must check e and result_, because cloners may return NULL
    // in case of errors.

    e->accept(*this);
# ifndef COMPILATION_MODE_SPACE
    if (result_ && !result_->original_get())
      result_->original_set(e->original_get());
# endif
    if (loc_)
      result_->location_set(loc_.get());
    if (use_map_)
      map_[(long)e] = result_;
  }


  void
  Cloner::visit (const ast::And* e)
  {
    const loc& location = e->location_get ();
    const exps_type& children = recurse_collection(e->children_get ());
    And* res = new And (location, children);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Assign* e)
  {
    const loc& location = e->location_get ();
    const rExp& what = recurse (e->what_get ());
    const rExp& value = recurse (e->value_get ());
    const boost::optional<modifiers_type>& modifiers = recurse (e->modifiers_get ());
    Assign* res = new Assign (location, what, value, modifiers);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Assignment* e)
  {
    const loc& location = e->location_get ();
    const rLValue& what = recurse (e->what_get ());
    const rExp& value = recurse (e->value_get ());
    Assignment* res = new Assignment (location, what, value);
    result_ = res;
  }

  void
  Cloner::visit (const ast::At* e)
  {
    const loc& location = e->location_get ();
    const flavor_type& flavor = e->flavor_get ();
    const loc& flavor_location = e->flavor_location_get ();
    bool sync = e->sync_get ();
    const rExp& cond = recurse (e->cond_get ());
    const rExp& body = recurse (e->body_get ());
    rExp onleave = recurse (e->onleave_get ());
    rExp duration = recurse (e->duration_get ());
    At* res = new At (location, flavor, flavor_location, sync, cond, body, onleave, duration);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Binding* e)
  {
    const loc& location = e->location_get ();
    const rLValue& what = recurse (e->what_get ());
    bool constant = e->constant_get ();
    Binding* res = new Binding (location, what);
    res->constant_set(constant);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Break* e)
  {
    const loc& location = e->location_get ();
    Break* res = new Break (location);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Call* e)
  {
    const loc& location = e->location_get ();
    exps_type* arguments = maybe_recurse_collection(e->arguments_get ());
    const rExp& target = recurse (e->target_get ());
    const libport::Symbol& name = e->name_get ();
    Call* res = new Call (location, arguments, target, name);
    result_ = res;
  }

  void
  Cloner::visit (const ast::CallMsg* e)
  {
    const loc& location = e->location_get ();
    CallMsg* res = new CallMsg (location);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Catch* e)
  {
    const loc& location = e->location_get ();
    rMatch match = recurse (e->match_get ());
    const rExp& body = recurse (e->body_get ());
    Catch* res = new Catch (location, match, body);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Class* e)
  {
    const loc& location = e->location_get ();
    const rLValue& what = recurse (e->what_get ());
    exps_type* protos = maybe_recurse_collection(e->protos_get ());
    const rExp& content = recurse (e->content_get ());
    bool is_package = e->is_package_get ();
    Class* res = new Class (location, what, protos, content, is_package);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Continue* e)
  {
    const loc& location = e->location_get ();
    Continue* res = new Continue (location);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Declaration* e)
  {
    const loc& location = e->location_get ();
    const rLValue& what = recurse (e->what_get ());
    const rExp& value = recurse (e->value_get ());
    bool constant = e->constant_get ();
    Declaration* res = new Declaration (location, what, value);
    res->constant_set(constant);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Decrementation* e)
  {
    const loc& location = e->location_get ();
    const rLValue& exp = recurse (e->exp_get ());
    bool post = e->post_get ();
    Decrementation* res = new Decrementation (location, exp, post);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Dictionary* e)
  {
    const loc& location = e->location_get ();
    const dictionary_elts_type& value = recurse_collection(e->value_get ());
    Dictionary* res = new Dictionary (location, value);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Do* e)
  {
    const loc& location = e->location_get ();
    const rExp& body = recurse (e->body_get ());
    const rExp& target = recurse (e->target_get ());
    Do* res = new Do (location, body, target);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Emit* e)
  {
    const loc& location = e->location_get ();
    const rExp& event = recurse (e->event_get ());
    exps_type* arguments = maybe_recurse_collection(e->arguments_get ());
    rExp duration = recurse (e->duration_get ());
    Emit* res = new Emit (location, event, arguments, duration);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Event* e)
  {
    const loc& location = e->location_get ();
    const rExp& exp = recurse (e->exp_get ());
    Event* res = new Event (location, exp);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Finally* e)
  {
    const loc& location = e->location_get ();
    const rExp& body = recurse (e->body_get ());
    const rExp& finally = recurse (e->finally_get ());
    Finally* res = new Finally (location, body, finally);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Float* e)
  {
    result_ = ast::rFloat(const_cast<ast::Float*>(e));
  }

  void
  Cloner::visit (const ast::Foreach* e)
  {
    const loc& location = e->location_get ();
    const flavor_type& flavor = e->flavor_get ();
    const rLocalDeclaration& index = recurse (e->index_get ());
    const rExp& list = recurse (e->list_get ());
    const rScope& body = recurse (e->body_get ());
    Foreach* res = new Foreach (location, flavor, index, list, body);
    result_ = res;
  }

  void
  Cloner::visit (const ast::If* e)
  {
    const loc& location = e->location_get ();
    const rExp& test = recurse (e->test_get ());
    const rScope& thenclause = recurse (e->thenclause_get ());
    const rScope& elseclause = recurse (e->elseclause_get ());
    If* res = new If (location, test, thenclause, elseclause);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Implicit* e)
  {
    result_ = ast::rImplicit(const_cast<ast::Implicit*>(e));
  }

  void
  Cloner::visit (const ast::Incrementation* e)
  {
    const loc& location = e->location_get ();
    const rLValue& exp = recurse (e->exp_get ());
    bool post = e->post_get ();
    Incrementation* res = new Incrementation (location, exp, post);
    result_ = res;
  }

  void
  Cloner::visit (const ast::List* e)
  {
    const loc& location = e->location_get ();
    exps_type* value = new exps_type(recurse_collection(e->value_get ()));
    List* res = new List (location, value);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Local* e)
  {
    const loc& location = e->location_get ();
    const libport::Symbol& name = e->name_get ();
    exps_type* arguments = maybe_recurse_collection(e->arguments_get ());
    unsigned depth = e->depth_get ();
    const rLocalDeclaration& declaration = recurse (e->declaration_get ());
    Local* res = new Local (location, name, arguments, depth);
    res->declaration_set(declaration);
    result_ = res;
  }

  void
  Cloner::visit (const ast::LocalAssignment* e)
  {
    const loc& location = e->location_get ();
    const libport::Symbol& what = e->what_get ();
    const rExp& value = recurse (e->value_get ());
    unsigned local_index = e->local_index_get ();
    unsigned depth = e->depth_get ();
    const rLocalDeclaration& declaration = recurse (e->declaration_get ());
    LocalAssignment* res = new LocalAssignment (location, what, value, depth);
    res->local_index_set(local_index);
    res->declaration_set(declaration);
    result_ = res;
  }

  void
  Cloner::visit (const ast::LocalDeclaration* e)
  {
    const loc& location = e->location_get ();
    const libport::Symbol& what = e->what_get ();
    const rExp& value = recurse (e->value_get ());
    unsigned local_index = e->local_index_get ();
    bool constant = e->constant_get ();
    bool list = e->list_get ();
    bool is_import = e->is_import_get ();
    bool is_star = e->is_star_get ();
    const rExp& type = recurse (e->type_get ());
    LocalDeclaration* res = new LocalDeclaration (location, what, value);
    res->local_index_set(local_index);
    res->constant_set(constant);
    res->list_set(list);
    res->is_import_set(is_import);
    res->is_star_set(is_star);
    res->type_set(type);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Match* e)
  {
    const loc& location = e->location_get ();
    const rExp& pattern = recurse (e->pattern_get ());
    rExp guard = recurse (e->guard_get ());
    rExp bindings = recurse (e->bindings_get ());
    Match* res = new Match (location, pattern, guard);
    res->bindings_set(bindings);
    result_ = res;
  }

  void
  Cloner::visit (const ast::MetaArgs* e)
  {
    const loc& location = e->location_get ();
    const rLValue& lvalue = recurse (e->lvalue_get ());
    unsigned id = e->id_get ();
    MetaArgs* res = new MetaArgs (location, lvalue, id);
    result_ = res;
  }

  void
  Cloner::visit (const ast::MetaCall* e)
  {
    const loc& location = e->location_get ();
    exps_type* arguments = maybe_recurse_collection(e->arguments_get ());
    const rExp& target = recurse (e->target_get ());
    unsigned id = e->id_get ();
    MetaCall* res = new MetaCall (location, arguments, target, id);
    result_ = res;
  }

  void
  Cloner::visit (const ast::MetaExp* e)
  {
    const loc& location = e->location_get ();
    unsigned id = e->id_get ();
    MetaExp* res = new MetaExp (location, id);
    result_ = res;
  }

  void
  Cloner::visit (const ast::MetaId* e)
  {
    const loc& location = e->location_get ();
    exps_type* arguments = maybe_recurse_collection(e->arguments_get ());
    unsigned id = e->id_get ();
    MetaId* res = new MetaId (location, arguments, id);
    result_ = res;
  }

  void
  Cloner::visit (const ast::MetaLValue* e)
  {
    const loc& location = e->location_get ();
    exps_type* arguments = maybe_recurse_collection(e->arguments_get ());
    unsigned id = e->id_get ();
    MetaLValue* res = new MetaLValue (location, arguments, id);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Nary* e)
  {
    const loc& location = e->location_get ();
    exps_type* children = new exps_type(recurse_collection(e->children_get ()));
    Nary* res = new Nary (location);
    res->children_set(children);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Noop* e)
  {
    const loc& location = e->location_get ();
    const rExp& body = recurse (e->body_get ());
    Noop* res = new Noop (location, body);
    result_ = res;
  }

  void
  Cloner::visit (const ast::OpAssignment* e)
  {
    const loc& location = e->location_get ();
    const rLValue& what = recurse (e->what_get ());
    const rExp& value = recurse (e->value_get ());
    const libport::Symbol& op = e->op_get ();
    OpAssignment* res = new OpAssignment (location, what, value, op);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Pipe* e)
  {
    const loc& location = e->location_get ();
    const exps_type& children = recurse_collection(e->children_get ());
    Pipe* res = new Pipe (location, children);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Property* e)
  {
    const loc& location = e->location_get ();
    const rExp& owner = recurse (e->owner_get ());
    const libport::Symbol& name = e->name_get ();
    Property* res = new Property (location, owner, name);
    result_ = res;
  }

  void
  Cloner::visit (const ast::PropertyWrite* e)
  {
    const loc& location = e->location_get ();
    const rExp& owner = recurse (e->owner_get ());
    const libport::Symbol& name = e->name_get ();
    const rExp& value = recurse (e->value_get ());
    PropertyWrite* res = new PropertyWrite (location, owner, name, value);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Return* e)
  {
    const loc& location = e->location_get ();
    rExp value = recurse (e->value_get ());
    Return* res = new Return (location, value);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Routine* e)
  {
    const loc& location = e->location_get ();
    bool closure = e->closure_get ();
    local_declarations_type* formals = maybe_recurse_collection(e->formals_get ());
    rScope body = recurse (e->body_get ());
    local_declarations_type* local_variables = maybe_recurse_collection(e->local_variables_get ());
    local_declarations_type* captured_variables = maybe_recurse_collection(e->captured_variables_get ());
    unsigned local_size = e->local_size_get ();
    bool uses_call = e->uses_call_get ();
    bool has_imports = e->has_imports_get ();
    Routine* res = new Routine (location, closure, formals, body);
    res->local_variables_set(local_variables);
    res->captured_variables_set(captured_variables);
    res->local_size_set(local_size);
    res->uses_call_set(uses_call);
    res->has_imports_set(has_imports);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Scope* e)
  {
    const loc& location = e->location_get ();
    const rExp& body = recurse (e->body_get ());
    Scope* res = new Scope (location, body);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Stmt* e)
  {
    const loc& location = e->location_get ();
    const flavor_type& flavor = e->flavor_get ();
    const rExp& expression = recurse (e->expression_get ());
    Stmt* res = new Stmt (location, flavor, expression);
    result_ = res;
  }

  void
  Cloner::visit (const ast::String* e)
  {
    result_ = ast::rString(const_cast<ast::String*>(e));
  }

  void
  Cloner::visit (const ast::Subscript* e)
  {
    const loc& location = e->location_get ();
    exps_type* arguments = maybe_recurse_collection(e->arguments_get ());
    const rExp& target = recurse (e->target_get ());
    Subscript* res = new Subscript (location, arguments, target);
    result_ = res;
  }

  void
  Cloner::visit (const ast::TaggedStmt* e)
  {
    const loc& location = e->location_get ();
    const rExp& tag = recurse (e->tag_get ());
    const rExp& exp = recurse (e->exp_get ());
    TaggedStmt* res = new TaggedStmt (location, tag, exp);
    result_ = res;
  }

  void
  Cloner::visit (const ast::This* e)
  {
    const loc& location = e->location_get ();
    This* res = new This (location);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Throw* e)
  {
    const loc& location = e->location_get ();
    rExp value = recurse (e->value_get ());
    Throw* res = new Throw (location, value);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Try* e)
  {
    const loc& location = e->location_get ();
    const rScope& body = recurse (e->body_get ());
    const catches_type& handlers = recurse_collection(e->handlers_get ());
    rScope elseclause = recurse (e->elseclause_get ());
    Try* res = new Try (location, body, handlers, elseclause);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Unscope* e)
  {
    const loc& location = e->location_get ();
    unsigned count = e->count_get ();
    Unscope* res = new Unscope (location, count);
    result_ = res;
  }

  void
  Cloner::visit (const ast::Watch* e)
  {
    const loc& location = e->location_get ();
    const rExp& exp = recurse (e->exp_get ());
    Watch* res = new Watch (location, exp);
    result_ = res;
  }

  void
  Cloner::visit (const ast::While* e)
  {
    const loc& location = e->location_get ();
    const flavor_type& flavor = e->flavor_get ();
    const rExp& test = recurse (e->test_get ());
    const rScope& body = recurse (e->body_get ());
    While* res = new While (location, flavor, test, body);
    result_ = res;
  }

} // namespace ast
