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
 ** \file ast/property.hxx
 ** \brief Inline methods of ast::Property.
 */

#ifndef AST_PROPERTY_HXX
# define AST_PROPERTY_HXX

# include <ast/property.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Property::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    PropertyAction::serialize(ser);
  }

  template <typename T>
  Property::Property(libport::serialize::ISerializer<T>& ser)
    : PropertyAction(ser)
  {
    LIBPORT_USE(ser);
  }
#endif


} // namespace ast

#endif // !AST_PROPERTY_HXX
