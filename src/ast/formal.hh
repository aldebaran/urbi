/*
 * Copyright (C) 2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_AST_FORMAL_HH
# define URBI_AST_FORMAL_HH

# include <libport/attributes.hh>
# include <libport/symbol.hh>

# include <ast/fwd.hh>

namespace ast
{
  class Formal
  {
  public:
    Formal();
    Formal(libport::Symbol name, rExp def = 0);
    Formal(libport::Symbol name, bool list);
    ATTRIBUTE_R(libport::Symbol, name);
    ATTRIBUTE_RW(rExp, def);
    ATTRIBUTE_R(bool, list);
  };
  typedef std::vector<Formal> Formals;

  std::ostream& operator<<(std::ostream& o, const ast::Formal& f);
  std::ostream& operator<<(std::ostream& o, const ast::Formals& f);
}

#endif
