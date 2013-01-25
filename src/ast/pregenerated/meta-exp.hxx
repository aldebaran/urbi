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
 ** \file ast/meta-exp.hxx
 ** \brief Inline methods of ast::MetaExp.
 */

#ifndef AST_META_EXP_HXX
# define AST_META_EXP_HXX

# include <ast/meta-exp.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  MetaExp::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< unsigned >("id", id_);
  }

  template <typename T>
  MetaExp::MetaExp(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    id_ = ser.template unserialize< unsigned >("id");
  }
#endif

  inline const unsigned&
  MetaExp::id_get () const
  {
    return id_;
  }
  inline unsigned&
  MetaExp::id_get ()
  {
    return id_;
  }


} // namespace ast

#endif // !AST_META_EXP_HXX
