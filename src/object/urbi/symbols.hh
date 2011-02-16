/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_URBI_SYMBOLS_HH
# define OBJECT_URBI_SYMBOLS_HH

// Using SYMBOL in these objects means exposing the corresponding
// static variables in libuobject's interface (using URBI_SDK_API) for
// a poor gain.
//
// Indeed, most of these symbols are used just to register the slots,
// and it might be about as costly to fetch a variable from a shared
// libary as it is to build the Symbol.
//
// Beware that it means that magical conversion of DOLLAR_ and so
// forth does not work.

# include <object/symbols.hh>
# undef SYMBOL_
# define SYMBOL_(Name) ::libport::Symbol(#Name)

#endif // !OBJECT_URBI_SYMBOLS_HH
