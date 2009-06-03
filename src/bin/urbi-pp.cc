#include <iostream>

#include <ast/serialize.hh>
#include <ast/print.hh>

int
main()
{
  ast::rAst ast = ast::unserialize(std::cin);
  std::cout << *ast;
}
