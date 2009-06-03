#include <iostream>

#include <ast/serialize.hh>
#include <parser/transform.hh>

int
main()
{
  ast::rAst ast = ast::unserialize(std::cin);
  ast::rAst res = parser::transform(ast);
  ast::serialize(res, std::cout);
}
