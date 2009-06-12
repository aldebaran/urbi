#include <ast/serialize.hh>
#include <bin/arg.hh>
#include <parser/parse.hh>

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
    input->read(buf, BUFSIZ);
    buf[input->gcount()] = 0;
    source += buf;
  }

  ast::rAst res = parser::parse(source, __HERE__)->ast_get();
  ast::serialize(res, *output);
}
