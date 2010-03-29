/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

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
