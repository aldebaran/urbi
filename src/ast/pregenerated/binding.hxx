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
 ** \file ast/binding.hxx
 ** \brief Inline methods of ast::Binding.
 */

#ifndef AST_BINDING_HXX
# define AST_BINDING_HXX

# include <ast/binding.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Binding::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    LValue::serialize(ser);
    ser.template serialize< rLValue >("what", what_);
    ser.template serialize< bool >("constant", constant_);
  }

  template <typename T>
  Binding::Binding(libport::serialize::ISerializer<T>& ser)
    : LValue(ser)
  {
    LIBPORT_USE(ser);
    what_ = ser.template unserialize< rLValue >("what");
    constant_ = ser.template unserialize< bool >("constant");
  }
#endif

  inline const rLValue&
  Binding::what_get () const
  {
    return what_;
  }
  inline rLValue&
  Binding::what_get ()
  {
    return what_;
  }

  inline const bool&
  Binding::constant_get () const
  {
    return constant_;
  }
  inline void
  Binding::constant_set (bool constant)
  {
    constant_ = constant;
  }


} // namespace ast

#endif // !AST_BINDING_HXX
