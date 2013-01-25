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
 ** \file ast/throw.hxx
 ** \brief Inline methods of ast::Throw.
 */

#ifndef AST_THROW_HXX
# define AST_THROW_HXX

# include <ast/throw.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Throw::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< rExp >("value", value_);
  }

  template <typename T>
  Throw::Throw(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    value_ = ser.template unserialize< rExp >("value");
  }
#endif

  inline const rExp&
  Throw::value_get () const
  {
    return value_;
  }
  inline rExp&
  Throw::value_get ()
  {
    return value_;
  }


} // namespace ast

#endif // !AST_THROW_HXX
