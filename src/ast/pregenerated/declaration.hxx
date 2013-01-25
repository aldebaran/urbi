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
 ** \file ast/declaration.hxx
 ** \brief Inline methods of ast::Declaration.
 */

#ifndef AST_DECLARATION_HXX
# define AST_DECLARATION_HXX

# include <ast/declaration.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Declaration::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Write::serialize(ser);
    ser.template serialize< bool >("constant", constant_);
  }

  template <typename T>
  Declaration::Declaration(libport::serialize::ISerializer<T>& ser)
    : Write(ser)
  {
    LIBPORT_USE(ser);
    constant_ = ser.template unserialize< bool >("constant");
  }
#endif

  inline const bool&
  Declaration::constant_get () const
  {
    return constant_;
  }
  inline void
  Declaration::constant_set (bool constant)
  {
    constant_ = constant;
  }


} // namespace ast

#endif // !AST_DECLARATION_HXX
