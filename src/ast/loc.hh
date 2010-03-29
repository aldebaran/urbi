/*
 * Copyright (C) 2007-2010, Gostai S.A.S.
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

# include <kernel/config.h>

# define LOCATION_HERE					\
  ::ast::loc(new libport::Symbol(__FILE__ + sizeof("../" SRCDIR "/src/") - 1), \
             __LINE__)

namespace ast
{
  typedef yy::location loc;
}

#endif // !AST_LOC_HH
