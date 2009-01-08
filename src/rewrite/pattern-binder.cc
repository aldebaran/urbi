#include <libport/lexical-cast.hh>

#include <ast/exps-type.hh>
#include <ast/parametric-ast.hh>
#include <parser/ast-factory.hh>
#include <rewrite/pattern-binder.hh>

namespace rewrite
{
  using parser::ast_string;

  ast::rExp
  PatternBinder::to_binding(ast::rConstExp original)
  {
    PARAMETRIC_AST(rewrite, "Pattern.Binding.new(%exp:1)");
    rewrite % ast_string(original->location_get(), next_name());
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
    return exp(get % pattern_ % ast_string(ast::loc(), next_name()));
  }

  PatternBinder::PatternBinder(ast::rLValue pattern,
                               const ast::loc& loc,
                               bool assign)
    : bindings_(new ast::Pipe(loc, ast::exps_type()))
    , pattern_(pattern)
    , i_(0)
    , assign_(assign)
  {}

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

  void
  PatternBinder::visit(const ast::Call* call)
  {
    if (call->arguments_get() || !assign_)
    {
      super_type::visit(call);
      return;
    }
    i_++;
    result_ = to_binding(call);
    bind(call, false);
  }

  void
  PatternBinder::visit(const ast::Property* prop)
  {
    ast::loc loc = prop->location_get();

    i_++;
    result_ = to_binding(prop);
    bind(prop, false);
  }

  void
  PatternBinder::visit(const ast::Subscript* sub)
  {
    i_++;
    result_ = to_binding(sub);
    bind(sub, false);
  }

  ast::rPipe
  PatternBinder::bindings_get() const
  {
    return bindings_;
  }
}
