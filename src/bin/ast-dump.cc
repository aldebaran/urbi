#include <iostream>

#include <libport/sysexits.hh>

#include <ast/dot-print.hh>
#include <ast/nary.hh>
#include <binder/bind.hh>
#include <flower/flow.hh>
#include <parser/parse.hh>
#include <rewrite/rewrite.hh>

using namespace ast;
using namespace parser;

static const int sz = 4096;

static void
usage()
{
  std::cout <<
    "Dump ast in dot format.  Ast is dumped at each step of the\n"
    "transformation in a different file descriptor:\n"
    "  \n"
    "  3: parsing\n"
    "  4: flowing\n"
    "  5: desugaring\n"
    "  6: rescoping\n"
    "  7: binding\n"
    "  \n"
    "For instance, you might use it like this to see ast after\n"
    "desugaring:\n"
    "  \n"
    "  _build/src/ast-dump <foo.u 4> ast.dot && dotty ast.dot\n"
    "or, with zsh\n"
    "  dotty =(_build/src/ast-dump <foo.u 4>&1)\n";
  exit (EX_OK);
}

static void
print(rAst ast)
{
  static int fd = 2;
  ++fd;
  std::stringstream output;
  ast::dot_print(ast, output);
  write(fd, output.str().c_str(), output.str().length());
}

int
main(int argc, const char*[])
{
  if (argc != 1)
    usage();

  char buf[sz + 1];
  std::string source;
  while (!std::cin.eof())
  {
    std::cin.read(buf, sz);
    buf[std::cin.gcount()] = 0;
    source += buf;
  }

  rAst res = parse(source, __HERE__)->ast_get();

  print(res);

  res = flower::flow(res);
  print(res);

  res = rewrite::desugar(res);
  print(res);

  res = rewrite::rescope(res);
  print(res);

  res = binder::bind(res);
  print(res);
}
