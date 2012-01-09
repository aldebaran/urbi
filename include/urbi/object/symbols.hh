/*
 * Copyright (C) 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file urbi/object/symbols.hh
 ** \brief Frequently used symbol names.
 */

#ifndef OBJECT_SYMBOLS_HH
# define OBJECT_SYMBOLS_HH

# include <libport/config.h>
# include <libport/symbol.hh>
# include <urbi/export.hh>

/* Symbols are internalized, that is to say, we keep a single
   representative, and use only it to denote all the equal symbols
   (the "FlyWeight" design pattern).  It is there quite useful to
   predeclare the symbols we use, so that at runtime we don't have to
   recompute the single representative.

   Therefore, declare here all the symbols we use somewhere in the C++
   code.

   Use SYMBOL when the argument is exactly the literal you need.

   Use SYMBOL_ in a macro (so you want to use the literal provided
   once cpp is run, but you don't want the macro parameter name to be
   considered a literal symbol).

   Use SYMBOL_EXPAND when the literal needs to go throw CPP expansion
   (e.g., it includes __LINE__ or whatever).
*/

// Without secondary expansion.
# define SYMBOL             SYMBOL_
// With secondary expansion.
# define SYMBOL_EXPAND(Sym) SYMBOL_(Sym)


# if defined LIBPORT_SYMBOLS_PRECOMPILED

/*----------------------.
| Symbols Precompiled.  |
`----------------------*/

#  define SYMBOL_(Sym) ::object::symbol_ ## Sym

#  include <urbi/object/precompiled-symbols.hh>

namespace object
{

# define SYMBOL_DECLARE(Name, Value)		\
  URBI_SDK_API extern libport::Symbol symbol_ ## Name

  SYMBOLS_APPLY(SYMBOL_DECLARE);

# undef SYMBOL_DECLARE

} // namespace object

# else // ! LIBPORT_SYMBOLS_PRECOMPILED


/*--------------------------.
| Symbols Not Precompiled.  |
`--------------------------*/

namespace object
{
  URBI_SDK_API libport::Symbol symbol_get(const std::string& s = "");
}

#  define SYMBOL_(Sym) ::object::symbol_get(#Sym)

# endif // ! LIBPORT_SYMBOLS_PRECOMPILED

#endif // !OBJECT_SYMBOLS_HH
