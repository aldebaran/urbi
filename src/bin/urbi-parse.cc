#include <iostream>

#include <ast/serialize.hh>
#include <parser/parse.hh>

int
main()
{
  // FIXME: Don't read the whole entry, parse the stream.
  char buf[BUFSIZ + 1];
  std::string source;
  while (!std::cin.eof())
  {
    std::cin.read(buf, BUFSIZ);
    buf[std::cin.gcount()] = 0;
    source += buf;
  }

  ast::rAst res = parser::parse(source, __HERE__)->ast_get();
  ast::serialize(res, std::cout);
}
