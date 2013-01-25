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
 ** \file ast/meta-call.hxx
 ** \brief Inline methods of ast::MetaCall.
 */

#ifndef AST_META_CALL_HXX
# define AST_META_CALL_HXX

# include <ast/meta-call.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  MetaCall::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    LValueArgs::serialize(ser);
    ser.template serialize< rExp >("target", target_);
    ser.template serialize< unsigned >("id", id_);
  }

  template <typename T>
  MetaCall::MetaCall(libport::serialize::ISerializer<T>& ser)
    : LValueArgs(ser)
  {
    LIBPORT_USE(ser);
    target_ = ser.template unserialize< rExp >("target");
    id_ = ser.template unserialize< unsigned >("id");
  }
#endif

  inline const rExp&
  MetaCall::target_get () const
  {
    return target_;
  }
  inline rExp&
  MetaCall::target_get ()
  {
    return target_;
  }

  inline const unsigned&
  MetaCall::id_get () const
  {
    return id_;
  }
  inline unsigned&
  MetaCall::id_get ()
  {
    return id_;
  }


} // namespace ast

#endif // !AST_META_CALL_HXX
