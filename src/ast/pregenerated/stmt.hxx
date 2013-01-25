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
 ** \file ast/stmt.hxx
 ** \brief Inline methods of ast::Stmt.
 */

#ifndef AST_STMT_HXX
# define AST_STMT_HXX

# include <ast/stmt.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Stmt::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Flavored::serialize(ser);
    ser.template serialize< rExp >("expression", expression_);
  }

  template <typename T>
  Stmt::Stmt(libport::serialize::ISerializer<T>& ser)
    : Flavored(ser)
  {
    LIBPORT_USE(ser);
    expression_ = ser.template unserialize< rExp >("expression");
  }
#endif

  inline const rExp&
  Stmt::expression_get () const
  {
    return expression_;
  }
  inline rExp&
  Stmt::expression_get ()
  {
    return expression_;
  }


} // namespace ast

#endif // !AST_STMT_HXX
