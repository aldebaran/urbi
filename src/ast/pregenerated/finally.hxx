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
 ** \file ast/finally.hxx
 ** \brief Inline methods of ast::Finally.
 */

#ifndef AST_FINALLY_HXX
# define AST_FINALLY_HXX

# include <ast/finally.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Finally::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< rExp >("body", body_);
    ser.template serialize< rExp >("finally", finally_);
  }

  template <typename T>
  Finally::Finally(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    body_ = ser.template unserialize< rExp >("body");
    finally_ = ser.template unserialize< rExp >("finally");
  }
#endif

  inline const rExp&
  Finally::body_get () const
  {
    return body_;
  }
  inline rExp&
  Finally::body_get ()
  {
    return body_;
  }

  inline const rExp&
  Finally::finally_get () const
  {
    return finally_;
  }
  inline rExp&
  Finally::finally_get ()
  {
    return finally_;
  }


} // namespace ast

#endif // !AST_FINALLY_HXX
