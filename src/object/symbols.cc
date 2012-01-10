/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/symbols.cc
 ** \brief Frequently used symbol names.
 */

#include <urbi/object/symbols.hh>

#if defined LIBPORT_SYMBOLS_PRECOMPILED

namespace object
{

# define SYMBOL_DEFINE(Name, Value)		\
  libport::Symbol symbol_ ## Name (Value);
  SYMBOLS_APPLY(SYMBOL_DEFINE);
# undef SYMBOL_DEFINE

} // namespace object

#else // ! LIBPORT_SYMBOLS_PRECOMPILED

# include <boost/unordered_map.hpp>
# include <libport/debug.hh>
# include <string>
# include <urbi/object/precompiled-symbols.hh>

namespace object
{

  class SymbolMap
  {
  public:
    SymbolMap()
      : map_()
    {
      libport::Symbol value;
# define SYMBOL_DEFINE(Name, Value)		\
      value = libport::Symbol(Value);		\
      map_[#Name] = value;
      SYMBOLS_APPLY(SYMBOL_DEFINE);
# undef SYMBOL_DEFINE
    }

    /// Return the symbol associated to \a s if there is one.
    /// Otherwise create a symbol equal to \a s.
    /// \warning: Special characters (SP, LT, etc.) are *not*
    /// handled in this case.
    libport::Symbol
    operator[](const std::string& s)
    {
      map_type::iterator i = map_.find(s);
      if (i != map_.end())
        return i->second;
      else
      {
        GD_CATEGORY(Urbi.Symbols);
        GD_FINFO_DUMP("insert: %s", s);
        libport::Symbol res(s);
        map_[s] = res;
        return res;
      }
    }

  private:
    typedef boost::unordered_map<const std::string, libport::Symbol> map_type;
    map_type map_;
  };

  libport::Symbol
  symbol_get(const std::string& s)
  {
    static SymbolMap map;
    return map[s];
  }

} // namespace object

#endif // ! LIBPORT_SYMBOLS_PRECOMPILED
