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
 ** \file ast/catch.hxx
 ** \brief Inline methods of ast::Catch.
 */

#ifndef AST_CATCH_HXX
# define AST_CATCH_HXX

# include <ast/catch.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Catch::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< rMatch >("match", match_);
    ser.template serialize< rExp >("body", body_);
  }

  template <typename T>
  Catch::Catch(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    match_ = ser.template unserialize< rMatch >("match");
    body_ = ser.template unserialize< rExp >("body");
  }
#endif

  inline const rMatch&
  Catch::match_get () const
  {
    return match_;
  }
  inline rMatch&
  Catch::match_get ()
  {
    return match_;
  }

  inline const rExp&
  Catch::body_get () const
  {
    return body_;
  }
  inline rExp&
  Catch::body_get ()
  {
    return body_;
  }


} // namespace ast

#endif // !AST_CATCH_HXX
