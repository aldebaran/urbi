/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <ast/pretty-printer.hh>
#include <ast/print.hh>

namespace ast
{

  std::ostream&
  operator<< (std::ostream& o, const Ast& a)
  {
    PrettyPrinter p(o);
    p(&a);
    return o;
  }

} // namespace ast
