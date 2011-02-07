/*
 * Copyright (C) 2007-2011, Gostai S.A.S.
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

# define DECLARE_LOCATION_FILE                  \
  static /* const */ ::libport::Symbol          \
    _DECLARE_LOCATION_FILE_is_not_defined       \
    (__FILE__ + sizeof(__SRCDIR__ "/src/") - 1)

# define LOCATION_HERE                                          \
  ::ast::loc(&_DECLARE_LOCATION_FILE_is_not_defined, __LINE__)

namespace ast
{
  typedef yy::location loc;
}

#endif // !AST_LOC_HH
