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
 ** \file ast/property-action.hxx
 ** \brief Inline methods of ast::PropertyAction.
 */

#ifndef AST_PROPERTY_ACTION_HXX
# define AST_PROPERTY_ACTION_HXX

# include <ast/property-action.hh>

namespace ast
{

#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  PropertyAction::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    LValue::serialize(ser);
    ser.template serialize< rExp >("owner", owner_);
    ser.template serialize< libport::Symbol >("name", name_);
  }

  template <typename T>
  PropertyAction::PropertyAction(libport::serialize::ISerializer<T>& ser)
    : LValue(ser)
  {
    LIBPORT_USE(ser);
    owner_ = ser.template unserialize< rExp >("owner");
    name_ = ser.template unserialize< libport::Symbol >("name");
  }
#endif

  inline const rExp&
  PropertyAction::owner_get () const
  {
    return owner_;
  }
  inline rExp&
  PropertyAction::owner_get ()
  {
    return owner_;
  }

  inline const libport::Symbol&
  PropertyAction::name_get () const
  {
    return name_;
  }
  inline libport::Symbol&
  PropertyAction::name_get ()
  {
    return name_;
  }


} // namespace ast

#endif // !AST_PROPERTY_ACTION_HXX
