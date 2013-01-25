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
 ** \file ast/if.hxx
 ** \brief Inline methods of ast::If.
 */

#ifndef AST_IF_HXX
# define AST_IF_HXX

# include <ast/if.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  If::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< rExp >("test", test_);
    ser.template serialize< rScope >("thenclause", thenclause_);
    ser.template serialize< rScope >("elseclause", elseclause_);
  }

  template <typename T>
  If::If(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    test_ = ser.template unserialize< rExp >("test");
    thenclause_ = ser.template unserialize< rScope >("thenclause");
    elseclause_ = ser.template unserialize< rScope >("elseclause");
  }
#endif

  inline const rExp&
  If::test_get () const
  {
    return test_;
  }
  inline rExp&
  If::test_get ()
  {
    return test_;
  }

  inline const rScope&
  If::thenclause_get () const
  {
    return thenclause_;
  }
  inline rScope&
  If::thenclause_get ()
  {
    return thenclause_;
  }

  inline const rScope&
  If::elseclause_get () const
  {
    return elseclause_;
  }
  inline rScope&
  If::elseclause_get ()
  {
    return elseclause_;
  }


} // namespace ast

#endif // !AST_IF_HXX
