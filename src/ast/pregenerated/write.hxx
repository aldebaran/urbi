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
 ** \file ast/write.hxx
 ** \brief Inline methods of ast::Write.
 */

#ifndef AST_WRITE_HXX
# define AST_WRITE_HXX

# include <ast/write.hh>

namespace ast
{

#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Write::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< rLValue >("what", what_);
    ser.template serialize< rExp >("value", value_);
  }

  template <typename T>
  Write::Write(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    what_ = ser.template unserialize< rLValue >("what");
    value_ = ser.template unserialize< rExp >("value");
  }
#endif

  inline const rLValue&
  Write::what_get () const
  {
    return what_;
  }
  inline rLValue&
  Write::what_get ()
  {
    return what_;
  }

  inline const rExp&
  Write::value_get () const
  {
    return value_;
  }
  inline rExp&
  Write::value_get ()
  {
    return value_;
  }


} // namespace ast

#endif // !AST_WRITE_HXX
