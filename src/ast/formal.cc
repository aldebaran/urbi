/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <ast/exp.hh>
#include <ast/formal.hh>
#include <ast/print.hh>

namespace ast
{
  Formal::Formal()
    : name_()
    , def_()
  {}

  Formal::Formal(libport::Symbol name, rExp def)
    : name_(name)
    , def_(def)
  {}

  std::ostream&
  operator<<(std::ostream& o, const ast::Formal& f)
  {
    o << "var " << f.name_get();
    if (f.def_get())
      o << " = " << *f.def_get();
    return o;
  }

  std::ostream&
  operator<<(std::ostream& o, const ast::Formals& fs)
  {
    bool tail = false;
    foreach (const Formal& f, fs)
    {
      if (tail++)
        o << ", ";
      o << f;
    }
    return o;
  }
}
