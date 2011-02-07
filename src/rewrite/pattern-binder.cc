/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/lexical-cast.hh>

#include <ast/exps-type.hh>
#include <ast/parametric-ast.hh>
#include <ast/factory.hh>
#include <rewrite/pattern-binder.hh>

DECLARE_LOCATION_FILE;

namespace rewrite
{
  PatternBinder::PatternBinder(ast::rLValue pattern,
                               const ast::loc& loc)
    : bindings_(new ast::Pipe(loc, ast::exps_type()))
    , pattern_(pattern)
    , i_(0)
    , factory_(new ast::Factory())
  {}

  ast::rExp
  PatternBinder::to_binding(ast::rConstExp original)
  {
    PARAMETRIC_AST(rewrite, "Pattern.Binding.new(%exp:1)");
    rewrite % factory_->make_string(original->location_get(), next_name());
    ast::rExp res = exp(rewrite);
    res->original_set(original);
    return res;
  }

  void
  PatternBinder::bind(ast::rConstLValue what, bool decl)
  {
    PARAMETRIC_AST(bind, "%lvalue:1 = %exp:2");
    PARAMETRIC_AST(bind_decl, "var %lvalue:1 = %exp:2");
    bindings_->children_get().push_back
      (exp((decl ? bind_decl : bind)
           % const_cast<ast::LValue*>(what.get())
           % value_get()));
  }

  ast::rExp
  PatternBinder::value_get()
  {
    PARAMETRIC_AST(get, "%lvalue:1 . bindings[%exp:2]");
    return exp(get
               % pattern_
               % factory_->make_string(ast::loc(), next_name()));
  }

  libport::Symbol
  PatternBinder::next_name()
  {
    return libport::Symbol(boost::lexical_cast<std::string>(i_));
  }

  void
  PatternBinder::visit(const ast::Binding* binding)
  {
    i_++;
    result_ = to_binding(binding);
    bind(binding->what_get(), true);
  }

  ast::rPipe
  PatternBinder::bindings_get() const
  {
    return bindings_;
  }
}
