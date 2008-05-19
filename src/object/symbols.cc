/**
 ** \file object/symbols.cc
 ** \brief Frequently used symbol names.
 */

#include "object/symbols.hh"

#if defined SYMBOLS_PRECOMPILED

namespace object
{

# define SYMBOL_DEFINE(Name, Value)		\
  libport::Symbol symbol_ ## Name (Value);
  SYMBOLS_APPLY(SYMBOL_DEFINE);
# undef SYMBOL_DEFINE

} // namespace object

#else // ! SYMBOLS_PRECOMPILED

# include <map>
# include <string>
# include "object/precompiled-symbols.hh"

namespace object
{

  struct SymbolMap
  {
    typedef std::map<const std::string, libport::Symbol> map_type;
    map_type map_;
    SymbolMap()
      : map_()
    {
# define SYMBOL_DEFINE(Name, Value)		\
      map_[#Name] = libport::Symbol(Value);
      SYMBOLS_APPLY(SYMBOL_DEFINE);
# undef SYMBOL_DEFINE
    }
  };

  // FIXME: More const.
  libport::Symbol
  symbol_get(const std::string& s)
  {
    static SymbolMap map;
    return map.map_[s];
  }

} // namespace object

#endif // ! SYMBOLS_PRECOMPILED
