/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/separate.hh>
#include <libport/symbol.hh>

#include <ast/symbols-type.hh>

namespace std
{
  std::ostream&
  operator<<(std::ostream& o, const ast::symbols_type& ss)
  {
    return o << ::libport::separate(ss, ", ");
  }
}
