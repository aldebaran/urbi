/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef AST_ENUM_TYPES_HH
# define AST_ENUM_TYPES_HH

# include <iosfwd>
# include <vector>

# include <libport/fwd.hh>

namespace ast
{
  typedef std::pair<libport::Symbol,ast::rExp> enum_elt_type;
  typedef std::vector<enum_elt_type> enum_elts_type;
}

#endif // ! AST_ENUM_TYPES_HH
