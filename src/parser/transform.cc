#include <binder/bind.hh>
#include <flower/flow.hh>
#include <rewrite/rewrite.hh>
#include <parser/transform.hh>

#include <ast/nary.hh>

namespace parser
{
  template <typename T>
  libport::intrusive_ptr<T>
  transform(libport::intrusive_ptr<T> ast)
  {
    return binder::bind(rewrite::rewrite(ast::rConstNary(flower::flow(ast))));
  }

#define INST(Type)                              \
  template libport::intrusive_ptr<ast::Type>    \
  transform(libport::intrusive_ptr<ast::Type>); \

  INST(Ast);
  INST(Nary);
} // namespace parser
