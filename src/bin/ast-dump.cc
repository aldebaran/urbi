#include <iostream>

#include <libport/cstring>
#include <libport/sysexits.hh>

#include <ast/dot-print.hh>
#include <ast/nary.hh>
#include <binder/bind.hh>
#include <flower/flow.hh>
#include <parser/parse.hh>
#include <rewrite/rewrite.hh>

using namespace ast;
using namespace parser;

static void
usage()
{
  std::cout <<
    "usage: ast-dump FILE.u\n"
    "\n"
    "Dump ast in dot format.  Ast is dumped at each step of the\n"
    "transformation in a different file descriptor:\n"
    "\n"
    "  3: parsing\n"
    "  4: flowing\n"
    "  5: desugaring\n"
    "  6: rescoping\n"
    "  7: binding\n"
    "\n"
    "For instance, you might use it like this to see ast after\n"
    "desugaring:\n"
    "\n"
    "  _build/src/bin/ast-dump foo.u 4> ast.dot && dotty ast.dot\n"
    "\n"
    "or, with zsh\n"
    "\n"
    "  dotty =(_build/src/bin/ast-dump foo.u 4>&1)\n";
  exit (EX_OK);
}

static void
print(rAst ast)
{
  static int fd = 2;
  ++fd;
  std::stringstream output;
  ast::dot_print(ast, output);
  size_t s = write(fd, output.str().c_str(), output.str().length());
  assert(s == output.str().length());
  (void)s;
}

int
main(int argc, const char* argv[])
{
  if (argc != 2
      || libport::streq(argv[1], "-h")
      || libport::streq(argv[1], "--help"))
    usage();

  rAst res = parse_file(argv[1])->ast_get();
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
