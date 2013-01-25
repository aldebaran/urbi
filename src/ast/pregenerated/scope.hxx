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
 ** \file ast/scope.hxx
 ** \brief Inline methods of ast::Scope.
 */

#ifndef AST_SCOPE_HXX
# define AST_SCOPE_HXX

# include <ast/scope.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Scope::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< rExp >("body", body_);
  }

  template <typename T>
  Scope::Scope(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    body_ = ser.template unserialize< rExp >("body");
  }
#endif

  inline const rExp&
  Scope::body_get () const
  {
    return body_;
  }
  inline rExp&
  Scope::body_get ()
  {
    return body_;
  }
  inline void
  Scope::body_set (const rExp& body)
  {
    body_ = body;
  }


} // namespace ast

#endif // !AST_SCOPE_HXX
