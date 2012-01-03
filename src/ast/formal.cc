/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
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

  /*---------.
  | Formal.  |
  `---------*/
  Formal::Formal()
    : name_()
    , def_()
    , list_(false)
  {}

  Formal::Formal(libport::Symbol name, rExp def)
    : name_(name)
    , def_(def)
    , list_(false)
  {}

  Formal::Formal(libport::Symbol name, bool list)
    : name_(name)
    , def_()
    , list_(list)
  {}

  std::ostream&
  operator<<(std::ostream& o, const ast::Formal& f)
  {
    o << "var " << f.name_get();
    if (f.list_get())
      o << "[]";
    if (f.def_get())
      o << " = " << *f.def_get();
    return o;
  }

  /*----------.
  | Formals.  |
  `----------*/

  std::ostream&
  operator<<(std::ostream& o, const ast::Formals& fs)
  {
    return o << libport::separate(fs, ", ");
  }
}
