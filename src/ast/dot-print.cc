/*
 * Copyright (C) 2009-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <ast/dot-print.hh>
#include <ast/dot-printer.hh>

namespace ast
{
  void
  dot_print(rConstAst ast, std::ostream& stream, const std::string& title)
  {
    DotPrinter p(stream, title);
    p(ast.get());
  }
}
