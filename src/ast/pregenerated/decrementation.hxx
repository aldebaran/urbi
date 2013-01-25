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
 ** \file ast/decrementation.hxx
 ** \brief Inline methods of ast::Decrementation.
 */

#ifndef AST_DECREMENTATION_HXX
# define AST_DECREMENTATION_HXX

# include <ast/decrementation.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Decrementation::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Unary::serialize(ser);
  }

  template <typename T>
  Decrementation::Decrementation(libport::serialize::ISerializer<T>& ser)
    : Unary(ser)
  {
    LIBPORT_USE(ser);
  }
#endif


} // namespace ast

#endif // !AST_DECREMENTATION_HXX
