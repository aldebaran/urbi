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
 ** \file ast/tagged-stmt.hxx
 ** \brief Inline methods of ast::TaggedStmt.
 */

#ifndef AST_TAGGED_STMT_HXX
# define AST_TAGGED_STMT_HXX

# include <ast/tagged-stmt.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  TaggedStmt::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< rExp >("tag", tag_);
    ser.template serialize< rExp >("exp", exp_);
  }

  template <typename T>
  TaggedStmt::TaggedStmt(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    tag_ = ser.template unserialize< rExp >("tag");
    exp_ = ser.template unserialize< rExp >("exp");
  }
#endif

  inline const rExp&
  TaggedStmt::tag_get () const
  {
    return tag_;
  }
  inline rExp&
  TaggedStmt::tag_get ()
  {
    return tag_;
  }

  inline const rExp&
  TaggedStmt::exp_get () const
  {
    return exp_;
  }
  inline rExp&
  TaggedStmt::exp_get ()
  {
    return exp_;
  }


} // namespace ast

#endif // !AST_TAGGED_STMT_HXX
