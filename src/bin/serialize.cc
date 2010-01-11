/*
 * Copyright (C) 2010, Gostai S.A.S.
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

#include <ast/serializer.hh>
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
  const char* program = "_build/src/bin/serializer";
  std::cout <<
    "usage: serializer FILE.u\n"
    "\n"
    "Dump ast in urbiscript format.  Ast is dumped at each step of the\n"
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
    "  " << program << " foo.u 4> ast.u\n"
    "\n"
    "or,\n"
    "\n"
    "  function ast-dump ()\n"
    "  {\n"
    "    base=$1\n"
    "    " << program << " $base.u \\\n"
    "       3>$base.1.parse.u \\\n"
    "       4>$base.2.flow.u \\\n"
    "       5>$base.3.desugar.u \\\n"
    "       6>$base.4.rescope.u \\\n"
    "       7>$base.5.binding.u;\n"
    "  }\n";
  exit (EX_OK);
}

static void
print(rAst ast)
{
  static int fd = 2;
  ++fd;
  std::stringstream output;
  ast::SerializerPrinter sp(output);
  sp(ast.get());
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
