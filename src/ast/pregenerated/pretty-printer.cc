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

