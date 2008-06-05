/**
 ** \file object/symbols.hh
 ** \brief Frequently used symbol names.
 */

#ifndef OBJECT_SYMBOLS_HH
# define OBJECT_SYMBOLS_HH

# include <libport/symbol.hh>

# include <sdk/config.h>


# if defined SYMBOLS_PRECOMPILED

/*----------------------.
| Symbols Precompiled.  |
`----------------------*/

/* Symbols are internalized, that is to say, we keep a single
   representative, and use only it to denote all the equal symbols
   (the "FlyWeight" design pattern).  It is there quite useful to
   predeclare the symbols we use, so that at runtime we don't have to
   recompute the single representative.

   Therefore, declare here all the symbols we use somewhere in the C++
   code.  */

#  define SYMBOL(Sym) object::symbol_ ## Sym
#  include <object/precompiled-symbols.hh>

namespace object
{

# define SYMBOL_DECLARE(Name, Value)		\
  extern libport::Symbol symbol_ ## Name

  SYMBOLS_APPLY(SYMBOL_DECLARE);

# undef SYMBOL_DECLARE

} // namespace object

# else // ! SYMBOLS_PRECOMPILED


/*--------------------------.
| Symbols Not Precompiled.  |
`--------------------------*/

namespace object
{
  libport::Symbol symbol_get(const std::string& s);
}

#  define SYMBOL(Sym) ::object::symbol_get(#Sym)

# endif // ! SYMBOLS_PRECOMPILED

#endif // !OBJECT_SYMBOLS_HH
