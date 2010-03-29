/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/symbols.hh
 ** \brief Frequently used symbol names.
 */

#ifndef OBJECT_SYMBOLS_HH
# define OBJECT_SYMBOLS_HH

# include <libport/symbol.hh>

# include <kernel/config.h>
# include <urbi/export.hh>

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

#  define SYMBOL(Sym) ::object::symbol_ ## Sym
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
  URBI_SDK_API libport::Symbol symbol_get(const std::string& s = "");
}

#  define SYMBOL(Sym) ::object::symbol_get(#Sym)

# endif // ! SYMBOLS_PRECOMPILED

#endif // !OBJECT_SYMBOLS_HH
