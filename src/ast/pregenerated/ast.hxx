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
 ** \file ast/ast.hxx
 ** \brief Inline methods of ast::Ast.
 */

#ifndef AST_AST_HXX
# define AST_AST_HXX

# include <ast/ast.hh>

namespace ast
{

#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Ast::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
  }

  template <typename T>
  Ast::Ast(libport::serialize::ISerializer<T>& ser)
    
  {
    LIBPORT_USE(ser);
  }
#endif

  inline const loc&
  Ast::location_get () const
  {
    return location_;
  }
  inline void
  Ast::location_set (const loc& location)
  {
    location_ = location;
  }

  inline const rConstAst&
  Ast::original_get () const
  {
    return original_;
  }


} // namespace ast

#endif // !AST_AST_HXX
