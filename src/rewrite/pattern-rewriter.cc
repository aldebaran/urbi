#include <ast/parametric-ast.hh>
#include <parser/ast-factory.hh>
#include <rewrite/pattern-rewriter.hh>

namespace rewrite
{
  PatternRewriter::PatternRewriter()
    : decs_()
  {}

  PatternRewriter::~PatternRewriter()
  {}

  void
  PatternRewriter::visit(const ast::Binding* binding)
  {
    static ast::ParametricAst
      rewrite("Pattern.Binding.new(%exp:1, closure (v) { %exp:2 })");
    ast::loc loc = binding->location_get();
    libport::Symbol name = binding->name_get();
    rewrite
      % parser::ast_string(loc, name)
      % new ast::Assignment(loc, parser::ast_call(loc, name),
                            parser::ast_call(loc, SYMBOL(v)), 0);
    ast::rConstAst rewritten = exp(rewrite);
    operator() (rewritten.get());
    decs_.back().insert(name);
  }

  void
  PatternRewriter::visit(const ast::Nary* nary)
  {
    static ast::ParametricAst nil("nil");
    decs_.push_back(declarations_type());
    super_type::visit(nary);
    ast::rNary res = result_.unsafe_cast<ast::Nary>();
    foreach (const libport::Symbol& var, decs_.back())
      res->push_front(
        new ast::Declaration(nary->location_get(),
                             parser::ast_call(nary->location_get(), var),
                             exp(nil), 0),
        ast::flavor_pipe);
    decs_.pop_back();
  }
}
