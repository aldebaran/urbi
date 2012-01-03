/*
 * Copyright (C) 2009-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file parser/is-keyword.hh

#ifndef PARSER_IS_KEYWORD_HH
# define PARSER_IS_KEYWORD_HH

# include <kernel/config.h>
# include <libport/symbol.hh>

namespace parser
{
  /// Whether \a s is a reserved word, and therefore requires quotes
  /// when used as an identifier.
  bool is_keyword(libport::Symbol s);

#ifdef COMPILATION_MODE_SPACE
  // If space is tight, it is simpler to consider everything needs to
  // be quoted.  Define as inline to help the compiler in constant
  // propagation.
  inline
  bool
  is_keyword(libport::Symbol)
  {
    return true;
  }
#endif
}

#endif // !PARSER_IS_KEYWORD_HH
