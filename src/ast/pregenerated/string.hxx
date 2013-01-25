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
 ** \file ast/string.hxx
 ** \brief Inline methods of ast::String.
 */

#ifndef AST_STRING_HXX
# define AST_STRING_HXX

# include <ast/string.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  String::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< std::string >("value", value_);
  }

  template <typename T>
  String::String(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    value_ = ser.template unserialize< std::string >("value");
  }
#endif

  inline const std::string&
  String::value_get () const
  {
    return value_;
  }


} // namespace ast

#endif // !AST_STRING_HXX
