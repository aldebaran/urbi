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
 ** \file ast/return.hxx
 ** \brief Inline methods of ast::Return.
 */

#ifndef AST_RETURN_HXX
# define AST_RETURN_HXX

# include <ast/return.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Return::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< rExp >("value", value_);
  }

  template <typename T>
  Return::Return(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    value_ = ser.template unserialize< rExp >("value");
  }
#endif

  inline const rExp&
  Return::value_get () const
  {
    return value_;
  }
  inline rExp&
  Return::value_get ()
  {
    return value_;
  }


} // namespace ast

#endif // !AST_RETURN_HXX
