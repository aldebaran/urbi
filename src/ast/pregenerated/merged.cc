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
 ** \file ast/all.cc
 ** \brief All the AST class implementations.
 */


#include <ast/and.hcc>
#include <ast/assign.hcc>
#include <ast/assignment.hcc>
#include <ast/ast.hcc>
#include <ast/at.hcc>
#include <ast/binding.hcc>
#include <ast/break.hcc>
#include <ast/call.hcc>
#include <ast/call-msg.hcc>
#include <ast/catch.hcc>
#include <ast/class.hcc>
#include <ast/composite.hcc>
#include <ast/continue.hcc>
#include <ast/declaration.hcc>
#include <ast/decrementation.hcc>
#include <ast/dictionary.hcc>
#include <ast/do.hcc>
#include <ast/emit.hcc>
#include <ast/event.hcc>
#include <ast/exp.hcc>
#include <ast/finally.hcc>
#include <ast/flavored.hcc>
#include <ast/float.hcc>
#include <ast/foreach.hcc>
#include <ast/if.hcc>
#include <ast/implicit.hcc>
#include <ast/incrementation.hcc>
#include <ast/lvalue.hcc>
#include <ast/lvalue-args.hcc>
#include <ast/list.hcc>
#include <ast/local.hcc>
#include <ast/local-assignment.hcc>
#include <ast/local-declaration.hcc>
#include <ast/local-write.hcc>
#include <ast/match.hcc>
#include <ast/meta-args.hcc>
#include <ast/meta-call.hcc>
#include <ast/meta-exp.hcc>
#include <ast/meta-id.hcc>
#include <ast/meta-lvalue.hcc>
#include <ast/nary.hcc>
#include <ast/noop.hcc>
#include <ast/op-assignment.hcc>
#include <ast/pipe.hcc>
#include <ast/property.hcc>
#include <ast/property-action.hcc>
#include <ast/property-write.hcc>
#include <ast/return.hcc>
#include <ast/routine.hcc>
#include <ast/scope.hcc>
#include <ast/stmt.hcc>
#include <ast/string.hcc>
#include <ast/subscript.hcc>
#include <ast/tagged-stmt.hcc>
#include <ast/this.hcc>
#include <ast/throw.hcc>
#include <ast/try.hcc>
#include <ast/unary.hcc>
#include <ast/unscope.hcc>
#include <ast/watch.hcc>
#include <ast/while.hcc>
#include <ast/write.hcc>
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
 ** \file ast/dot-generator.cc
 ** \brief Implementation of ast::Cloner.
 */


#include <libport/lexical-cast.hh>
#include <ast/dot-printer.hh>
#include <ast/all.hh>

namespace ast
{
  DotPrinter::DotPrinter(std::ostream& output, const std::string& title)
    : output_(output)
    , id_(0)
    , ids_()
    , root_(true)
    , title_(title)
  {}

  DotPrinter::~DotPrinter()
  {}

  template <typename T>
  std::string
  escape(const T& e)
  {
    std::string res = string_cast(e);
    for (std::string::size_type i = res.find_first_of("<>");
         i != std::string::npos;
	 i = res.find_first_of("<>", i+4))
      res.replace(i, 1, res[i] == '<' ? "&lt;" : "&gt;");
    return res;
  }

  void DotPrinter::operator()(const ast::Ast* n)
  {
    if (!n)
      return;
    if (root_)
    {
      root_ = false;
      output_ << "digraph \"" << ast::escape(title_) << '"' << std::endl
              << "{" << std::endl
              << "  node [shape=\"record\"];" << std::endl;
      super_type::operator()(n);
      output_ << "}" << std::endl;
    }
    else
      super_type::operator()(n);
  }

  template<typename T>
  void
  DotPrinter::recurse(const T& c)
  {
    foreach (const typename T::value_type& elt, c)
      if (elt.get())
        operator()(elt.get());
  }

  template<>
  void
  DotPrinter::recurse<symbols_type>(const symbols_type&)
  {}

  template<>
  void
  DotPrinter::recurse<ast::modifiers_type>(const ast::modifiers_type&)
  {}

  template<>
  void
  DotPrinter::recurse<ast::dictionary_elts_type>(const ast::dictionary_elts_type&)
  {}

  template<typename T>
  void
  DotPrinter::recurse(const T* c)
  {
    if (c)
      recurse(*c);
  }

    void
  DotPrinter::visit(const And* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{And|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "children";
    recurse(n->children_get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Assign* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Assign|{" << ast::escape(n->location_get()) << " }|{modifiers: " << ast::escape(n->modifiers_get()) << "}}\"];" << std::endl;
    ids_.back().second = "what";
    operator() (n->what_get().get());
    ids_.back().second = "value";
    operator() (n->value_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Assignment* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Assignment|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "what";
    operator() (n->what_get().get());
    ids_.back().second = "value";
    operator() (n->value_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const At* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{At|{" << ast::escape(n->location_get()) << " }|{flavor: " << ast::escape(n->flavor_get()) << "|flavor_location: " << ast::escape(n->flavor_location_get()) << "|sync: " << ast::escape(n->sync_get()) << "}}\"];" << std::endl;
    ids_.back().second = "cond";
    operator() (n->cond_get().get());
    ids_.back().second = "body";
    operator() (n->body_get().get());
    ids_.back().second = "onleave";
    operator() (n->onleave_get().get());
    ids_.back().second = "duration";
    operator() (n->duration_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Binding* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Binding|{" << ast::escape(n->location_get()) << " }|{constant: " << ast::escape(n->constant_get()) << "}}\"];" << std::endl;
    ids_.back().second = "what";
    operator() (n->what_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Break* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Break|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Call* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Call|{" << ast::escape(n->location_get()) << " }|{name: " << ast::escape(n->name_get()) << "}}\"];" << std::endl;
    ids_.back().second = "arguments";
    recurse(n->arguments_get());
    ids_.back().second = "target";
    operator() (n->target_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const CallMsg* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{CallMsg|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Catch* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Catch|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "match";
    operator() (n->match_get().get());
    ids_.back().second = "body";
    operator() (n->body_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Class* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Class|{" << ast::escape(n->location_get()) << " }|{is_package: " << ast::escape(n->is_package_get()) << "}}\"];" << std::endl;
    ids_.back().second = "what";
    operator() (n->what_get().get());
    ids_.back().second = "protos";
    recurse(n->protos_get());
    ids_.back().second = "content";
    operator() (n->content_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Continue* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Continue|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Declaration* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Declaration|{" << ast::escape(n->location_get()) << " }|{constant: " << ast::escape(n->constant_get()) << "}}\"];" << std::endl;
    ids_.back().second = "what";
    operator() (n->what_get().get());
    ids_.back().second = "value";
    operator() (n->value_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Decrementation* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Decrementation|{" << ast::escape(n->location_get()) << " }|{post: " << ast::escape(n->post_get()) << "}}\"];" << std::endl;
    ids_.back().second = "exp";
    operator() (n->exp_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Dictionary* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Dictionary|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "value";
    recurse(n->value_get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Do* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Do|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "body";
    operator() (n->body_get().get());
    ids_.back().second = "target";
    operator() (n->target_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Emit* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Emit|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "event";
    operator() (n->event_get().get());
    ids_.back().second = "arguments";
    recurse(n->arguments_get());
    ids_.back().second = "duration";
    operator() (n->duration_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Event* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Event|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "exp";
    operator() (n->exp_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Finally* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Finally|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "body";
    operator() (n->body_get().get());
    ids_.back().second = "finally";
    operator() (n->finally_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Float* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Float|{" << ast::escape(n->location_get()) << " }|{value: " << ast::escape(n->value_get()) << "}}\"];" << std::endl;

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Foreach* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Foreach|{" << ast::escape(n->location_get()) << " }|{flavor: " << ast::escape(n->flavor_get()) << "}}\"];" << std::endl;
    ids_.back().second = "index";
    operator() (n->index_get().get());
    ids_.back().second = "list";
    operator() (n->list_get().get());
    ids_.back().second = "body";
    operator() (n->body_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const If* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{If|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "test";
    operator() (n->test_get().get());
    ids_.back().second = "thenclause";
    operator() (n->thenclause_get().get());
    ids_.back().second = "elseclause";
    operator() (n->elseclause_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Implicit* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Implicit|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Incrementation* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Incrementation|{" << ast::escape(n->location_get()) << " }|{post: " << ast::escape(n->post_get()) << "}}\"];" << std::endl;
    ids_.back().second = "exp";
    operator() (n->exp_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const List* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{List|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "value";
    recurse(n->value_get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Local* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Local|{" << ast::escape(n->location_get()) << " }|{name: " << ast::escape(n->name_get()) << "|depth: " << ast::escape(n->depth_get()) << "}}\"];" << std::endl;
    ids_.back().second = "arguments";
    recurse(n->arguments_get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const LocalAssignment* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{LocalAssignment|{" << ast::escape(n->location_get()) << " }|{what: " << ast::escape(n->what_get()) << "|local_index: " << ast::escape(n->local_index_get()) << "|depth: " << ast::escape(n->depth_get()) << "}}\"];" << std::endl;
    ids_.back().second = "value";
    operator() (n->value_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const LocalDeclaration* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{LocalDeclaration|{" << ast::escape(n->location_get()) << " }|{what: " << ast::escape(n->what_get()) << "|local_index: " << ast::escape(n->local_index_get()) << "|constant: " << ast::escape(n->constant_get()) << "|list: " << ast::escape(n->list_get()) << "|is_import: " << ast::escape(n->is_import_get()) << "|is_star: " << ast::escape(n->is_star_get()) << "}}\"];" << std::endl;
    ids_.back().second = "value";
    operator() (n->value_get().get());
    ids_.back().second = "type";
    operator() (n->type_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Match* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Match|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "pattern";
    operator() (n->pattern_get().get());
    ids_.back().second = "guard";
    operator() (n->guard_get().get());
    ids_.back().second = "bindings";
    operator() (n->bindings_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const MetaArgs* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{MetaArgs|{" << ast::escape(n->location_get()) << " }|{id: " << ast::escape(n->id_get()) << "}}\"];" << std::endl;
    ids_.back().second = "lvalue";
    operator() (n->lvalue_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const MetaCall* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{MetaCall|{" << ast::escape(n->location_get()) << " }|{id: " << ast::escape(n->id_get()) << "}}\"];" << std::endl;
    ids_.back().second = "arguments";
    recurse(n->arguments_get());
    ids_.back().second = "target";
    operator() (n->target_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const MetaExp* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{MetaExp|{" << ast::escape(n->location_get()) << " }|{id: " << ast::escape(n->id_get()) << "}}\"];" << std::endl;

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const MetaId* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{MetaId|{" << ast::escape(n->location_get()) << " }|{id: " << ast::escape(n->id_get()) << "}}\"];" << std::endl;
    ids_.back().second = "arguments";
    recurse(n->arguments_get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const MetaLValue* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{MetaLValue|{" << ast::escape(n->location_get()) << " }|{id: " << ast::escape(n->id_get()) << "}}\"];" << std::endl;
    ids_.back().second = "arguments";
    recurse(n->arguments_get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Nary* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Nary|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "children";
    recurse(n->children_get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Noop* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Noop|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "body";
    operator() (n->body_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const OpAssignment* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{OpAssignment|{" << ast::escape(n->location_get()) << " }|{op: " << ast::escape(n->op_get()) << "}}\"];" << std::endl;
    ids_.back().second = "what";
    operator() (n->what_get().get());
    ids_.back().second = "value";
    operator() (n->value_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Pipe* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Pipe|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "children";
    recurse(n->children_get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Property* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Property|{" << ast::escape(n->location_get()) << " }|{name: " << ast::escape(n->name_get()) << "}}\"];" << std::endl;
    ids_.back().second = "owner";
    operator() (n->owner_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const PropertyWrite* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{PropertyWrite|{" << ast::escape(n->location_get()) << " }|{name: " << ast::escape(n->name_get()) << "}}\"];" << std::endl;
    ids_.back().second = "owner";
    operator() (n->owner_get().get());
    ids_.back().second = "value";
    operator() (n->value_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Return* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Return|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "value";
    operator() (n->value_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Routine* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Routine|{" << ast::escape(n->location_get()) << " }|{closure: " << ast::escape(n->closure_get()) << "|local_size: " << ast::escape(n->local_size_get()) << "|uses_call: " << ast::escape(n->uses_call_get()) << "|has_imports: " << ast::escape(n->has_imports_get()) << "}}\"];" << std::endl;
    ids_.back().second = "formals";
    recurse(n->formals_get());
    ids_.back().second = "body";
    operator() (n->body_get().get());
    ids_.back().second = "local_variables";
    recurse(n->local_variables_get());
    ids_.back().second = "captured_variables";
    recurse(n->captured_variables_get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Scope* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Scope|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "body";
    operator() (n->body_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Stmt* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Stmt|{" << ast::escape(n->location_get()) << " }|{flavor: " << ast::escape(n->flavor_get()) << "}}\"];" << std::endl;
    ids_.back().second = "expression";
    operator() (n->expression_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const String* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{String|{" << ast::escape(n->location_get()) << " }|{value: " << ast::escape(n->value_get()) << "}}\"];" << std::endl;

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Subscript* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Subscript|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "arguments";
    recurse(n->arguments_get());
    ids_.back().second = "target";
    operator() (n->target_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const TaggedStmt* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{TaggedStmt|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "tag";
    operator() (n->tag_get().get());
    ids_.back().second = "exp";
    operator() (n->exp_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const This* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{This|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Throw* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Throw|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "value";
    operator() (n->value_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Try* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Try|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "body";
    operator() (n->body_get().get());
    ids_.back().second = "handlers";
    recurse(n->handlers_get());
    ids_.back().second = "elseclause";
    operator() (n->elseclause_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Unscope* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Unscope|{" << ast::escape(n->location_get()) << " }|{count: " << ast::escape(n->count_get()) << "}}\"];" << std::endl;

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const Watch* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{Watch|{" << ast::escape(n->location_get()) << " }}\"];" << std::endl;
    ids_.back().second = "exp";
    operator() (n->exp_get().get());

    ids_.pop_back();
  }

  void
  DotPrinter::visit(const While* n)
  {
    LIBPORT_USE(n);

    ++id_;
    if (!ids_.empty())
      output_ << "  node_" << ids_.back().first << " -> node_" << id_
              << " [label=\"" << ids_.back().second << "\"];" << std::endl;
    ids_.push_back(std::make_pair(id_, ""));
    output_ << "  node_" << id_ << " [label=\"{While|{" << ast::escape(n->location_get()) << " }|{flavor: " << ast::escape(n->flavor_get()) << "}}\"];" << std::endl;
    ids_.back().second = "test";
    operator() (n->test_get().get());
    ids_.back().second = "body";
    operator() (n->body_get().get());

    ids_.pop_back();
  }


}

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
 ** \file ast/pretty-printer.cc
 ** \brief Implementation of ast::PrettyPrinter.
 */

#include <ostream>

#include <ast/all.hh>
#include <ast/pretty-printer.hh>
#include <ast/print.hh>

#include <libport/foreach.hh>
#include <libport/escape.hh>
#include <libport/indent.hh>
#include <libport/pair.hh>

namespace ast
{

  PrettyPrinter::PrettyPrinter (std::ostream& ostr)
    : ostr_ (ostr)
  {
  }

  PrettyPrinter::~PrettyPrinter ()
  {
  }

  void
  PrettyPrinter::operator() (const Ast* e)
  {
    static bool desugar = getenv("URBI_DESUGAR");
    rConstAst original = e->original_get();
    if (!desugar && original)
      operator()(original.get());
    else
      super_type::operator()(e);
  }


  void
  PrettyPrinter::visit (const And* n)
  {
    LIBPORT_USE(n);
    ostr_ << libport::separate(n->children_get(), " & ");
  }

  void
  PrettyPrinter::visit (const Assign* n)
  {
    LIBPORT_USE(n);
    operator()(n->what_get().get());
    ostr_ << " = ";
    operator()(n->value_get().get());
    { if (n->modifiers_get()) { foreach (const modifiers_type::value_type& modifier, n->modifiers_get().get()) ostr_ << " " << modifier.first << ": " << *modifier.second; } }
  }

  void
  PrettyPrinter::visit (const At* n)
  {
    LIBPORT_USE(n);
    ostr_ << "at (";
    operator()(n->cond_get().get());
    ostr_ << ")";
    ostr_ << libport::iendl;
    operator()(n->body_get().get());
    ostr_ << libport::iendl;
    ostr_ << "onleave";
    ostr_ << libport::iendl;
    if (n->onleave_get()) operator()(n->onleave_get().get());
  }

  void
  PrettyPrinter::visit (const Binding* n)
  {
    LIBPORT_USE(n);
    ostr_ << "var ";
    operator()(n->what_get().get());
  }

  void
  PrettyPrinter::visit (const Break* n)
  {
    LIBPORT_USE(n);
    ostr_ << "break";
  }

  void
  PrettyPrinter::visit (const Call* n)
  {
    LIBPORT_USE(n);
    { if (!(n->target_get()->implicit())) ostr_ << *n->target_get() << "."; }
    { n->name_get().print_escaped(ostr_); }
    { visit(static_cast<const LValueArgs*>(n)); }
  }

  void
  PrettyPrinter::visit (const CallMsg* n)
  {
    LIBPORT_USE(n);
    ostr_ << "call";
  }

  void
  PrettyPrinter::visit (const Catch* n)
  {
    LIBPORT_USE(n);
    ostr_ << "catch";
    { if (rConstMatch m = n->match_get()) ostr_ << " (" << *m << ")"; }
    ostr_ << libport::iendl;
    ostr_ << "{";
    ostr_ << libport::incendl;
    operator()(n->body_get().get());
    ostr_ << libport::decendl;
    ostr_ << "}";
  }

  void
  PrettyPrinter::visit (const Class* n)
  {
    LIBPORT_USE(n);
    ostr_ << "class ";
    operator()(n->what_get().get());
    { if (const exps_type* protos = n->protos_get()) ostr_ << ": " << libport::separate(*protos, ", "); }
    ostr_ << libport::iendl;
    ostr_ << "{";
    ostr_ << libport::incendl;
    operator()(n->content_get().get());
    ostr_ << libport::decendl;
    ostr_ << "}";
  }

  void
  PrettyPrinter::visit (const Continue* n)
  {
    LIBPORT_USE(n);
    ostr_ << "continue";
  }

  void
  PrettyPrinter::visit (const Declaration* n)
  {
    LIBPORT_USE(n);
    ostr_ << "var ";
    { visit(static_cast<const Write*>(n)); }
  }

  void
  PrettyPrinter::visit (const Decrementation* n)
  {
    LIBPORT_USE(n);
    { if (n->post_get()) ostr_ << *n->exp_get() << "--"; else ostr_ << "--" << *n->exp_get(); }
  }

  void
  PrettyPrinter::visit (const Dictionary* n)
  {
    LIBPORT_USE(n);
    ostr_ << "[";
    { if (n->value_get().empty()) ostr_ << " => "; else { ostr_ << libport::incindent; foreach (dictionary_elts_type::value_type exp, n->value_get()) ostr_  << libport::iendl << *exp.first .get() << " => " << *exp.second.get() << ","; ostr_ << libport::decendl; } }
    ostr_ << "]";
  }

  void
  PrettyPrinter::visit (const Do* n)
  {
    LIBPORT_USE(n);
    ostr_ << "do (";
    operator()(n->target_get().get());
    ostr_ << ") ";
    { visit(static_cast<const Scope*>(n)); }
  }

  void
  PrettyPrinter::visit (const Emit* n)
  {
    LIBPORT_USE(n);
    operator()(n->event_get().get());
    ostr_ << "!";
    { if (const exps_type* args = n->arguments_get()) ostr_ << "(" << libport::separate(*args, ", ") << ")"; }
    { if (rConstExp duration = n->duration_get()) ostr_ << " ~ " << *duration; }
  }

  void
  PrettyPrinter::visit (const Event* n)
  {
    LIBPORT_USE(n);
    ostr_ << "$event(";
    operator()(n->exp_get().get());
    ostr_ << ")";
  }

  void
  PrettyPrinter::visit (const Finally* n)
  {
    LIBPORT_USE(n);
    ostr_ << "try";
    ostr_ << libport::iendl;
    operator()(n->body_get().get());
    ostr_ << libport::iendl;
    ostr_ << "finally";
    ostr_ << libport::iendl;
    ostr_ << "{";
    ostr_ << libport::incendl;
    operator()(n->finally_get().get());
    ostr_ << libport::decendl;
    ostr_ << "}";
  }

  void
  PrettyPrinter::visit (const Flavored* n)
  {
    LIBPORT_USE(n);
    { if (n->flavor_get() != flavor_none && n->flavor_get() != flavor_semicolon) ostr_ << n->flavor_get(); }
  }

  void
  PrettyPrinter::visit (const Float* n)
  {
    LIBPORT_USE(n);
    ostr_ << n->value_get();
  }

  void
  PrettyPrinter::visit (const Foreach* n)
  {
    LIBPORT_USE(n);
    ostr_ << "for";
    { visit(static_cast<const Flavored*>(n)); }
    ostr_ << " (var ";
    { n->index_get()->what_get().print_escaped(ostr_); }
    ostr_ << " : ";
    operator()(n->list_get().get());
    ostr_ << ")";
    ostr_ << libport::iendl;
    operator()(n->body_get().get());
  }

  void
  PrettyPrinter::visit (const If* n)
  {
    LIBPORT_USE(n);
    ostr_ << "if (";
    operator()(n->test_get().get());
    ostr_ << ")";
    ostr_ << libport::iendl;
    operator()(n->thenclause_get().get());
    { if (!n->elseclause_get()->empty()) { ostr_ << libport::iendl << "else"; if (n->elseclause_get()->body_get().is_a<If>()) ostr_ << " " << *n->elseclause_get()->body_get(); else ostr_ << libport::iendl << *n->elseclause_get(); } }
  }

  void
  PrettyPrinter::visit (const Implicit* n)
  {
    LIBPORT_USE(n);
    ostr_ << "<IMPLICIT>";
  }

  void
  PrettyPrinter::visit (const Incrementation* n)
  {
    LIBPORT_USE(n);
    { if (n->post_get()) ostr_ << *n->exp_get() << "++"; else ostr_ << "++" << *n->exp_get(); }
  }

  void
  PrettyPrinter::visit (const LValueArgs* n)
  {
    LIBPORT_USE(n);
    { if (n->arguments_get()) ostr_ << "(" << libport::incindent << libport::separate(*n->arguments_get(), ", ") << libport::decindent << ")"; }
  }

  void
  PrettyPrinter::visit (const List* n)
  {
    LIBPORT_USE(n);
    ostr_ << "[";
    ostr_ << libport::separate(n->value_get(), ", ");
    ostr_ << "]";
  }

  void
  PrettyPrinter::visit (const Local* n)
  {
    LIBPORT_USE(n);
    ostr_ << n->name_get();
    { if (n->arguments_get()) ostr_ << "(" << libport::incindent << libport::separate(*n->arguments_get(), ", ") << libport::decindent << ")"; }
  }

  void
  PrettyPrinter::visit (const LocalAssignment* n)
  {
    LIBPORT_USE(n);
    { visit(static_cast<const LocalWrite*>(n)); }
  }

  void
  PrettyPrinter::visit (const LocalDeclaration* n)
  {
    LIBPORT_USE(n);
    { if (n->constant_get()) ostr_ << "const "; }
    ostr_ << "var ";
    { visit(static_cast<const LocalWrite*>(n)); }
  }

  void
  PrettyPrinter::visit (const LocalWrite* n)
  {
    LIBPORT_USE(n);
    ostr_ << n->what_get();
    ostr_ << " /* idx: ";
    ostr_ << n->local_index_get();
    ostr_ << " */";
    { if (n->value_get()) ostr_ << " = " << *n->value_get(); }
  }

  void
  PrettyPrinter::visit (const Match* n)
  {
    LIBPORT_USE(n);
    { ostr_ << *n->pattern_get(); if (rConstExp guard = n->guard_get()) ostr_ << " if " << *guard; }
  }

  void
  PrettyPrinter::visit (const MetaArgs* n)
  {
    LIBPORT_USE(n);
    operator()(n->lvalue_get().get());
    ostr_ << "(%exps:";
    ostr_ << n->id_get();
    ostr_ << ")";
  }

  void
  PrettyPrinter::visit (const MetaCall* n)
  {
    LIBPORT_USE(n);
    operator()(n->target_get().get());
    ostr_ << ".%id:";
    ostr_ << n->id_get();
    ostr_ << " ";
    { visit(static_cast<const LValueArgs*>(n)); }
  }

  void
  PrettyPrinter::visit (const MetaExp* n)
  {
    LIBPORT_USE(n);
    ostr_ << "%exp:";
    ostr_ << n->id_get();
  }

  void
  PrettyPrinter::visit (const MetaId* n)
  {
    LIBPORT_USE(n);
    ostr_ << "%id:";
    ostr_ << n->id_get();
  }

  void
  PrettyPrinter::visit (const MetaLValue* n)
  {
    LIBPORT_USE(n);
    ostr_ << "%lvalue:";
    ostr_ << n->id_get();
  }

  void
  PrettyPrinter::visit (const Nary* n)
  {
    LIBPORT_USE(n);
    ostr_ << n->children_get();
  }

  void
  PrettyPrinter::visit (const Noop* n)
  {
    LIBPORT_USE(n);
    ostr_ << "{}";
  }

  void
  PrettyPrinter::visit (const OpAssignment* n)
  {
    LIBPORT_USE(n);
    operator()(n->what_get().get());
    ostr_ << " ";
    ostr_ << n->op_get();
    ostr_ << " ";
    operator()(n->value_get().get());
  }

  void
  PrettyPrinter::visit (const Pipe* n)
  {
    LIBPORT_USE(n);
    ostr_ << libport::separate(n->children_get(), " | ");
  }

  void
  PrettyPrinter::visit (const PropertyAction* n)
  {
    LIBPORT_USE(n);
    operator()(n->owner_get().get());
    ostr_ << "->";
    ostr_ << n->name_get();
  }

  void
  PrettyPrinter::visit (const PropertyWrite* n)
  {
    LIBPORT_USE(n);
    operator()(n->owner_get().get());
    ostr_ << "->";
    ostr_ << n->name_get();
    ostr_ << " = ";
    operator()(n->value_get().get());
  }

  void
  PrettyPrinter::visit (const Return* n)
  {
    LIBPORT_USE(n);
    ostr_ << "return";
    { if (n->value_get()) ostr_ << ' ' << *n->value_get(); }
  }

  void
  PrettyPrinter::visit (const Routine* n)
  {
    LIBPORT_USE(n);
    ostr_ << (n->closure_get() ? "closure" : "function");
    ostr_ << " ";
    { if (n->formals_get()) ostr_ << "(" << *n->formals_get() << ") "; }
    if (n->body_get()) operator()(n->body_get().get());
  }

  void
  PrettyPrinter::visit (const Scope* n)
  {
    LIBPORT_USE(n);
    ostr_ << "{";
    { if (n->single()) ostr_ << ' ' << *n->body_get() << ' '; else if (!n->body_get()->empty()) ostr_ << libport::incendl << *n->body_get() << libport::decendl; }
    ostr_ << "}";
  }

  void
  PrettyPrinter::visit (const Stmt* n)
  {
    LIBPORT_USE(n);
    operator()(n->expression_get().get());
    { if (n->flavor_get() != flavor_none) ostr_ << n->flavor_get(); }
  }

  void
  PrettyPrinter::visit (const String* n)
  {
    LIBPORT_USE(n);
    ostr_ << '"';
    ostr_ << libport::escape(n->value_get(), '"');
    ostr_ << '"';
  }

  void
  PrettyPrinter::visit (const Subscript* n)
  {
    LIBPORT_USE(n);
    operator()(n->target_get().get());
    ostr_ << "[";
    if (n->arguments_get()) ostr_ << *n->arguments_get();
    ostr_ << "]";
  }

  void
  PrettyPrinter::visit (const TaggedStmt* n)
  {
    LIBPORT_USE(n);
    operator()(n->tag_get().get());
    ostr_ << ": ";
    operator()(n->exp_get().get());
  }

  void
  PrettyPrinter::visit (const This* n)
  {
    LIBPORT_USE(n);
    ostr_ << "this";
  }

  void
  PrettyPrinter::visit (const Throw* n)
  {
    LIBPORT_USE(n);
    ostr_ << "throw";
    { if (rConstExp m = n->value_get()) ostr_ << " " << *m; }
  }

  void
  PrettyPrinter::visit (const Try* n)
  {
    LIBPORT_USE(n);
    ostr_ << "try";
    ostr_ << libport::iendl;
    operator()(n->body_get().get());
    ostr_ << libport::iendl;
    ostr_ << n->handlers_get();
    { if (n->elseclause_get()) ostr_ << libport::iendl << "else" << libport::iendl << *n->elseclause_get(); }
  }

  void
  PrettyPrinter::visit (const Unscope* n)
  {
    LIBPORT_USE(n);
    ostr_ << "%unscope:";
    ostr_ << n->count_get();
  }

  void
  PrettyPrinter::visit (const Watch* n)
  {
    LIBPORT_USE(n);
    ostr_ << "watch(";
    operator()(n->exp_get().get());
    ostr_ << ")";
  }

  void
  PrettyPrinter::visit (const While* n)
  {
    LIBPORT_USE(n);
    ostr_ << "while";
    { visit(static_cast<const Flavored*>(n)); }
    ostr_ << " (";
    operator()(n->test_get().get());
    ostr_ << ")";
    ostr_ << libport::iendl;
    operator()(n->body_get().get());
  }

  void
  PrettyPrinter::visit (const Write* n)
  {
    LIBPORT_USE(n);
    operator()(n->what_get().get());
    { if (const rExp& value = n->value_get()) ostr_ << " = " << *value; }
  }

} // namespace ast

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

