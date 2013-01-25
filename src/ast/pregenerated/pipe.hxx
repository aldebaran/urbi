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
 ** \file ast/pipe.hxx
 ** \brief Inline methods of ast::Pipe.
 */

#ifndef AST_PIPE_HXX
# define AST_PIPE_HXX

# include <ast/pipe.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Pipe::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Composite::serialize(ser);
  }

  template <typename T>
  Pipe::Pipe(libport::serialize::ISerializer<T>& ser)
    : Composite(ser)
  {
    LIBPORT_USE(ser);
  }
#endif


} // namespace ast

#endif // !AST_PIPE_HXX
