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
 ** \file ast/try.hxx
 ** \brief Inline methods of ast::Try.
 */

#ifndef AST_TRY_HXX
# define AST_TRY_HXX

# include <ast/try.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Try::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< rScope >("body", body_);
    ser.template serialize< catches_type >("handlers", handlers_);
    ser.template serialize< rScope >("elseclause", elseclause_);
  }

  template <typename T>
  Try::Try(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    body_ = ser.template unserialize< rScope >("body");
    handlers_ = ser.template unserialize< catches_type >("handlers");
    elseclause_ = ser.template unserialize< rScope >("elseclause");
  }
#endif

  inline const rScope&
  Try::body_get () const
  {
    return body_;
  }
  inline rScope&
  Try::body_get ()
  {
    return body_;
  }

  inline const catches_type&
  Try::handlers_get () const
  {
    return handlers_;
  }
  inline catches_type&
  Try::handlers_get ()
  {
    return handlers_;
  }

  inline const rScope&
  Try::elseclause_get () const
  {
    return elseclause_;
  }
  inline rScope&
  Try::elseclause_get ()
  {
    return elseclause_;
  }


} // namespace ast

#endif // !AST_TRY_HXX
