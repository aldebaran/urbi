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
 ** \file ast/unscope.hxx
 ** \brief Inline methods of ast::Unscope.
 */

#ifndef AST_UNSCOPE_HXX
# define AST_UNSCOPE_HXX

# include <ast/unscope.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Unscope::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< unsigned >("count", count_);
  }

  template <typename T>
  Unscope::Unscope(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    count_ = ser.template unserialize< unsigned >("count");
  }
#endif

  inline const unsigned&
  Unscope::count_get () const
  {
    return count_;
  }


} // namespace ast

#endif // !AST_UNSCOPE_HXX
