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

#include <libport/cerrno>
#include <libport/cstdio>
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
  // Not argv[0] because we need the libtool wrapper.
  const char* program = "_build/src/bin/ast-dump";
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
    "  " << program << " foo.u 4> ast.dot && dotty ast.dot\n"
    "\n"
    "or, with zsh\n"
    "\n"
    "  dotty =(_build/src/bin/ast-dump foo.u 4>&1)\n"
    "\n"
    "or,\n"
    "\n"
    "  function ast-dump ()\n"
    "  {\n"
    "    base=$1\n"
    "    " << program << " $base.u \\\n"
    "       3>$base.1.parse.dot \\\n"
    "       4>$base.2.flow.dot \\\n"
    "       5>$base.3.desugar.dot \\\n"
    "       6>$base.4.rescope.dot \\\n"
    "       7>$base.5.binding.dot;\n"
    "  }\n";
  exit (EX_OK);
}

static void
print(const std::string& name, rAst ast)
{
  static int fd = 2;
  ++fd;
  std::stringstream output;
  ast::dot_print(ast, output, name);
  ssize_t s = write(fd, output.str().c_str(), output.str().length());
  if (0 <= s)
  {
    assert_eq(static_cast<size_t>(s), output.str().length());
    std::cerr << "filled fd " << fd << std::endl;
  }
  else if (errno != EBADF)
    perror("write");
  else
    std::cerr << "skipped fd " << fd << std::endl;
}

int
main(int argc, const char* argv[])
{
  if (argc != 2
      || libport::streq(argv[1], "-h")
      || libport::streq(argv[1], "--help"))
    usage();

  rAst res = parse_file(argv[1]);
  print("parse", res);

  res = flower::flow<Ast>(res);
  print("flow", res);

  res = rewrite::desugar(res);
  print("rewrite", res);

  res = rewrite::rescope(res);
  print("rescope", res);

  res = binder::bind(res);
  print("bind", res);
}
