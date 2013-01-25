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
 ** \file ast/do.hxx
 ** \brief Inline methods of ast::Do.
 */

#ifndef AST_DO_HXX
# define AST_DO_HXX

# include <ast/do.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Do::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Scope::serialize(ser);
    ser.template serialize< rExp >("target", target_);
  }

  template <typename T>
  Do::Do(libport::serialize::ISerializer<T>& ser)
    : Scope(ser)
  {
    LIBPORT_USE(ser);
    target_ = ser.template unserialize< rExp >("target");
  }
#endif

  inline const rExp&
  Do::target_get () const
  {
    return target_;
  }
  inline rExp&
  Do::target_get ()
  {
    return target_;
  }


} // namespace ast

#endif // !AST_DO_HXX
