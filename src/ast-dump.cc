/**
 *  Dump ast in dot format.  Ast is dumped at each step of the
 *  transformation in a different file descriptor:
 *
 *  3: parsing
 *  4: desugaring
 *  5: rescoping
 *  6: flowing
 *  7: binding
 *
 *  For instance, you might use it like this to see ast after
 *  desugaring:
 *
 *  _build/src/ast-dump 4> ast.dot && dot -Tpng ast.dot > ast.png
 *
 */

#include <iostream>

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
print(rAst ast)
{
  static int fd = 2;
  ++fd;
  std::stringstream output;
  ast::dot_print(ast, output);
  write(fd, output.str().c_str(), output.str().length());
}

int
main()
{
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

  res = rewrite::desugar(res);
  print(res);

  res = rewrite::rescope(res);
  print(res);

  res = flower::flow(res);
  print(res);

  res = binder::bind(res);
  print(res);
}
