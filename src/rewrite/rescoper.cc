#include <parser/ast-factory.hh>
#include <rewrite/rescoper.hh>


namespace rewrite
{
  Rescoper::Rescoper()
  {}

  Rescoper::~Rescoper()
  {}

  void Rescoper::visit(ast::rConstAnd a)
  {
    ast::rAnd res = new ast::And(a->location_get(), ast::exps_type());
    foreach (ast::rExp child, a->children_get())
      // Wrap every children in a closure
      res->children_get().push_back(recurse(parser::ast_closure(child)));
    result_ = res;
  }
}
