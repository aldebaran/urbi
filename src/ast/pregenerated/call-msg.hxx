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
 ** \file ast/call-msg.hxx
 ** \brief Inline methods of ast::CallMsg.
 */

#ifndef AST_CALL_MSG_HXX
# define AST_CALL_MSG_HXX

# include <ast/call-msg.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  CallMsg::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
  }

  template <typename T>
  CallMsg::CallMsg(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
  }
#endif


} // namespace ast

#endif // !AST_CALL_MSG_HXX
