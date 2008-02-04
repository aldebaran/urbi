/**
 ** \file object/symbols.cc
 ** \brief Frequently used symbol names.
 */

#include "object/symbols.hh"

namespace object
{

# define SYMBOL_DEFINE(Name, Value)		\
  libport::Symbol symbol_ ## Name (#Value);

  SYMBOLS_APPLY(SYMBOL_DEFINE);

# undef SYMBOL_DEFINE

} // namespace object
