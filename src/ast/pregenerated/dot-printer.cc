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

