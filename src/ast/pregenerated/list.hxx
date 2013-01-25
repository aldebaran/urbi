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
 ** \file ast/list.hxx
 ** \brief Inline methods of ast::List.
 */

#ifndef AST_LIST_HXX
# define AST_LIST_HXX

# include <ast/list.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  List::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< exps_type* >("value", value_);
  }

  template <typename T>
  List::List(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    value_ = ser.template unserialize< exps_type* >("value");
  }
#endif

  inline const exps_type&
  List::value_get () const
  {
    return *value_;
  }
  inline exps_type&
  List::value_get ()
  {
    return *value_;
  }


} // namespace ast

#endif // !AST_LIST_HXX
