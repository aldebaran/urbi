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
 ** \file ast/op-assignment.hxx
 ** \brief Inline methods of ast::OpAssignment.
 */

#ifndef AST_OP_ASSIGNMENT_HXX
# define AST_OP_ASSIGNMENT_HXX

# include <ast/op-assignment.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  OpAssignment::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Write::serialize(ser);
    ser.template serialize< libport::Symbol >("op", op_);
  }

  template <typename T>
  OpAssignment::OpAssignment(libport::serialize::ISerializer<T>& ser)
    : Write(ser)
  {
    LIBPORT_USE(ser);
    op_ = ser.template unserialize< libport::Symbol >("op");
  }
#endif

  inline const libport::Symbol&
  OpAssignment::op_get () const
  {
    return op_;
  }
  inline libport::Symbol&
  OpAssignment::op_get ()
  {
    return op_;
  }


} // namespace ast

#endif // !AST_OP_ASSIGNMENT_HXX
