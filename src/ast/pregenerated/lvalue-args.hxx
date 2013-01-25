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
 ** \file ast/lvalue-args.hxx
 ** \brief Inline methods of ast::LValueArgs.
 */

#ifndef AST_LVALUE_ARGS_HXX
# define AST_LVALUE_ARGS_HXX

# include <ast/lvalue-args.hh>

namespace ast
{

#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  LValueArgs::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    LValue::serialize(ser);
    ser.template serialize< exps_type* >("arguments", arguments_);
  }

  template <typename T>
  LValueArgs::LValueArgs(libport::serialize::ISerializer<T>& ser)
    : LValue(ser)
  {
    LIBPORT_USE(ser);
    arguments_ = ser.template unserialize< exps_type* >("arguments");
  }
#endif

  inline const exps_type*
  LValueArgs::arguments_get () const
  {
    return arguments_;
  }
  inline exps_type*
  LValueArgs::arguments_get ()
  {
    return arguments_;
  }
  inline void
  LValueArgs::arguments_set (exps_type* arguments)
  {
    delete arguments_;
    arguments_ = arguments;
  }


} // namespace ast

#endif // !AST_LVALUE_ARGS_HXX
