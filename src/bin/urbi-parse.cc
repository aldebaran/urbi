/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <ast/serialize.hh>
#include <bin/arg.hh>
#include <parser/parse.hh>

DECLARE_LOCATION_FILE;

int
main(int argc, char** argv)
{
  // FIXME: Don't read the whole entry, parse the stream.
  char buf[BUFSIZ + 1];
  std::string source;

  std::istream* input;
  std::ostream* output;
  get_io(input, output, argc, argv);

  while (!input->eof())
  {
    input->read(buf, sizeof buf);
    source.append(buf, input->gcount());
  }

  ast::rAst res = parser::parse(source, LOCATION_HERE)->ast_get();
  ast::serialize(res, *output);
}
