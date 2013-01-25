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
 ** \file ast/nary.hxx
 ** \brief Inline methods of ast::Nary.
 */

#ifndef AST_NARY_HXX
# define AST_NARY_HXX

# include <ast/nary.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Nary::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< exps_type* >("children", children_);
  }

  template <typename T>
  Nary::Nary(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    children_ = ser.template unserialize< exps_type* >("children");
  }
#endif

  inline const exps_type&
  Nary::children_get () const
  {
    return *children_;
  }
  inline exps_type&
  Nary::children_get ()
  {
    return *children_;
  }
  inline void
  Nary::children_set (exps_type* children)
  {
    delete children_;
    children_ = children;
  }


} // namespace ast

#endif // !AST_NARY_HXX
