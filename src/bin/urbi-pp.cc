#include <iostream>

#include <ast/serialize.hh>
#include <ast/print.hh>

#include <ast/serialize.hh>
#include <bin/arg.hh>
#include <parser/parse.hh>

int
main(int argc, char** argv)
{
  std::istream* input;
  std::ostream* output;
  get_io(input, output, argc, argv);

  ast::rAst ast = ast::unserialize(*input);
  *output << *ast;
}
