/*
 * Copyright (C) 2006-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// Generated, do not edit by hand.

/**
 ** \file ast/noop.hxx
 ** \brief Inline methods of ast::Noop.
 */

#ifndef AST_NOOP_HXX
# define AST_NOOP_HXX

# include <ast/noop.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Noop::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Scope::serialize(ser);
  }

  template <typename T>
  Noop::Noop(libport::serialize::ISerializer<T>& ser)
    : Scope(ser)
  {
    LIBPORT_USE(ser);
  }
#endif


} // namespace ast

#endif // !AST_NOOP_HXX
