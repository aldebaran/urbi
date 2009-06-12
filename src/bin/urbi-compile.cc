#include <iostream>

#include <ast/serialize.hh>
#include <bin/arg.hh>
#include <parser/transform.hh>

int
main(int argc, char** argv)
{
  std::istream* input;
  std::ostream* output;
  get_io(input, output, argc, argv);

  ast::rAst ast = ast::unserialize(*input);
  ast::serialize(parser::transform(ast), *output);
}
