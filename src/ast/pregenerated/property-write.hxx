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
 ** \file ast/property-write.hxx
 ** \brief Inline methods of ast::PropertyWrite.
 */

#ifndef AST_PROPERTY_WRITE_HXX
# define AST_PROPERTY_WRITE_HXX

# include <ast/property-write.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  PropertyWrite::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    PropertyAction::serialize(ser);
    ser.template serialize< rExp >("value", value_);
  }

  template <typename T>
  PropertyWrite::PropertyWrite(libport::serialize::ISerializer<T>& ser)
    : PropertyAction(ser)
  {
    LIBPORT_USE(ser);
    value_ = ser.template unserialize< rExp >("value");
  }
#endif

  inline const rExp&
  PropertyWrite::value_get () const
  {
    return value_;
  }
  inline rExp&
  PropertyWrite::value_get ()
  {
    return value_;
  }


} // namespace ast

#endif // !AST_PROPERTY_WRITE_HXX
