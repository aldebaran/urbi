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
 ** \file ast/float.hxx
 ** \brief Inline methods of ast::Float.
 */

#ifndef AST_FLOAT_HXX
# define AST_FLOAT_HXX

# include <ast/float.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Float::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< libport::ufloat >("value", value_);
  }

  template <typename T>
  Float::Float(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    value_ = ser.template unserialize< libport::ufloat >("value");
  }
#endif

  inline const libport::ufloat&
  Float::value_get () const
  {
    return value_;
  }


} // namespace ast

#endif // !AST_FLOAT_HXX
