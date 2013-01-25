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
 ** \file ast/call.hxx
 ** \brief Inline methods of ast::Call.
 */

#ifndef AST_CALL_HXX
# define AST_CALL_HXX

# include <ast/call.hh>

namespace ast
{

    inline bool Call::target_implicit() const
    {
      return target_->implicit();
    }


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Call::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    LValueArgs::serialize(ser);
    ser.template serialize< rExp >("target", target_);
    ser.template serialize< libport::Symbol >("name", name_);
  }

  template <typename T>
  Call::Call(libport::serialize::ISerializer<T>& ser)
    : LValueArgs(ser)
  {
    LIBPORT_USE(ser);
    target_ = ser.template unserialize< rExp >("target");
    name_ = ser.template unserialize< libport::Symbol >("name");
  }
#endif

  inline const rExp&
  Call::target_get () const
  {
    return target_;
  }
  inline rExp&
  Call::target_get ()
  {
    return target_;
  }
  inline void
  Call::target_set (const rExp& target)
  {
    target_ = target;
  }

  inline const libport::Symbol&
  Call::name_get () const
  {
    return name_;
  }
  inline libport::Symbol&
  Call::name_get ()
  {
    return name_;
  }


} // namespace ast

#endif // !AST_CALL_HXX
