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
 ** \file ast/flavored.hxx
 ** \brief Inline methods of ast::Flavored.
 */

#ifndef AST_FLAVORED_HXX
# define AST_FLAVORED_HXX

# include <ast/flavored.hh>

namespace ast
{

#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Flavored::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< flavor_type >("flavor", flavor_);
  }

  template <typename T>
  Flavored::Flavored(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    flavor_ = ser.template unserialize< flavor_type >("flavor");
  }
#endif

  inline const flavor_type&
  Flavored::flavor_get () const
  {
    return flavor_;
  }
  inline void
  Flavored::flavor_set (const flavor_type& flavor)
  {
    flavor_ = flavor;
  }


} // namespace ast

#endif // !AST_FLAVORED_HXX
