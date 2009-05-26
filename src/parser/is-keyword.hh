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

#ifdef OPTIMIZE_SPACE
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
