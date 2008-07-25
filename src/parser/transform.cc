#include <binder/bind.hh>
#include <flower/flow.hh>
#include <rewrite/rewrite.hh>
#include <parser/transform.hh>

#include <ast/nary.hh>

namespace parser
{
  ast::rNary
  transform(ast::rConstNary ast)
  {
    return binder::bind(flower::flow(rewrite::rewrite(ast)));
  }
} // namespace parser

