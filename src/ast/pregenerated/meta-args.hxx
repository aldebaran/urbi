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
 ** \file ast/meta-args.hxx
 ** \brief Inline methods of ast::MetaArgs.
 */

#ifndef AST_META_ARGS_HXX
# define AST_META_ARGS_HXX

# include <ast/meta-args.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  MetaArgs::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    LValue::serialize(ser);
    ser.template serialize< rLValue >("lvalue", lvalue_);
    ser.template serialize< unsigned >("id", id_);
  }

  template <typename T>
  MetaArgs::MetaArgs(libport::serialize::ISerializer<T>& ser)
    : LValue(ser)
  {
    LIBPORT_USE(ser);
    lvalue_ = ser.template unserialize< rLValue >("lvalue");
    id_ = ser.template unserialize< unsigned >("id");
  }
#endif

  inline const rLValue&
  MetaArgs::lvalue_get () const
  {
    return lvalue_;
  }
  inline rLValue&
  MetaArgs::lvalue_get ()
  {
    return lvalue_;
  }

  inline const unsigned&
  MetaArgs::id_get () const
  {
    return id_;
  }
  inline unsigned&
  MetaArgs::id_get ()
  {
    return id_;
  }


} // namespace ast

#endif // !AST_META_ARGS_HXX
