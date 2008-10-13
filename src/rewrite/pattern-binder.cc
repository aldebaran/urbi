#include <ast/parametric-ast.hh>
#include <parser/ast-factory.hh>
#include <rewrite/pattern-binder.hh>

namespace rewrite
{
  PatternBinder::PatternBinder(const ast::rComposite& container)
    : container_(container)
  {}

  void
  PatternBinder::visit(const ast::Binding* binding)
  {
    PARAMETRIC_AST(bind, "var %id:1 = '$pattern'.bindings[%exp:2]");

    libport::Symbol name = binding->name_get();
    container_->children_get().push_front
      (exp(bind % name % parser::ast_string(binding->location_get(), name)));
    super_type::visit(binding);
  }
}
