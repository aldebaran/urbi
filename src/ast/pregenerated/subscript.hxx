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
 ** \file ast/subscript.hxx
 ** \brief Inline methods of ast::Subscript.
 */

#ifndef AST_SUBSCRIPT_HXX
# define AST_SUBSCRIPT_HXX

# include <ast/subscript.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Subscript::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    LValueArgs::serialize(ser);
    ser.template serialize< rExp >("target", target_);
  }

  template <typename T>
  Subscript::Subscript(libport::serialize::ISerializer<T>& ser)
    : LValueArgs(ser)
  {
    LIBPORT_USE(ser);
    target_ = ser.template unserialize< rExp >("target");
  }
#endif

  inline const rExp&
  Subscript::target_get () const
  {
    return target_;
  }
  inline rExp&
  Subscript::target_get ()
  {
    return target_;
  }


} // namespace ast

#endif // !AST_SUBSCRIPT_HXX
