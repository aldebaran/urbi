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
 ** \file ast/while.hxx
 ** \brief Inline methods of ast::While.
 */

#ifndef AST_WHILE_HXX
# define AST_WHILE_HXX

# include <ast/while.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  While::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Flavored::serialize(ser);
    ser.template serialize< rExp >("test", test_);
    ser.template serialize< rScope >("body", body_);
  }

  template <typename T>
  While::While(libport::serialize::ISerializer<T>& ser)
    : Flavored(ser)
  {
    LIBPORT_USE(ser);
    test_ = ser.template unserialize< rExp >("test");
    body_ = ser.template unserialize< rScope >("body");
  }
#endif

  inline const rExp&
  While::test_get () const
  {
    return test_;
  }
  inline rExp&
  While::test_get ()
  {
    return test_;
  }

  inline const rScope&
  While::body_get () const
  {
    return body_;
  }
  inline rScope&
  While::body_get ()
  {
    return body_;
  }


} // namespace ast

#endif // !AST_WHILE_HXX
