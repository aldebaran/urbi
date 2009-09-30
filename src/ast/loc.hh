/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/**
 ** \file ast/loc.hh
 ** \brief Definition of ast::loc.
 */

#ifndef AST_LOC_HH
# define AST_LOC_HH

# include <libport/symbol.hh>

# include <parser/location.hh>

# define LOCATION_HERE					\
  ::ast::loc(new libport::Symbol(__FILE__), __LINE__)

namespace ast
{
  typedef yy::location loc;
}

#endif // !AST_LOC_HH
