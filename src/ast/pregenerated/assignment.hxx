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
 ** \file ast/assignment.hxx
 ** \brief Inline methods of ast::Assignment.
 */

#ifndef AST_ASSIGNMENT_HXX
# define AST_ASSIGNMENT_HXX

# include <ast/assignment.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Assignment::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Write::serialize(ser);
  }

  template <typename T>
  Assignment::Assignment(libport::serialize::ISerializer<T>& ser)
    : Write(ser)
  {
    LIBPORT_USE(ser);
  }
#endif


} // namespace ast

#endif // !AST_ASSIGNMENT_HXX
