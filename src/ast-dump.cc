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

#include <ast/dot-printer.hh>
#include <ast/nary.hh>

#include <binder/binder.hh>

#include <flower/flower.hh>

#include <parser/parse.hh>

#include <rewrite/desugarer.hh>
#include <rewrite/rescoper.hh>

using namespace ast;
using namespace parser;

static const int sz = 4096;

static void
print(rAst ast)
{
  static int fd = 2;
  ++fd;
  std::stringstream output;
  DotPrinter p(output);
  p(ast.get());
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

  rAst res = parse(source, __HERE__)->ast_get();;

  print(res);

  rewrite::Desugarer desugar;
  desugar(res.get());
  res = desugar.result_get();
  print(res);

  rewrite::Rescoper rescope;
  rescope(res.get());
  res = rescope.result_get();
  print(res);

  flower::Flower flow;
  res = ast::analyze(flow, res.unsafe_cast<ast::Nary>().get());
  print(res);

  binder::Binder bind;
  res = ast::analyze(bind, res.unsafe_cast<ast::Nary>().get());
  print(res);
}
