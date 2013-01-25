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
 ** \file ast/foreach.hxx
 ** \brief Inline methods of ast::Foreach.
 */

#ifndef AST_FOREACH_HXX
# define AST_FOREACH_HXX

# include <ast/foreach.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Foreach::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Flavored::serialize(ser);
    ser.template serialize< rLocalDeclaration >("index", index_);
    ser.template serialize< rExp >("list", list_);
    ser.template serialize< rScope >("body", body_);
  }

  template <typename T>
  Foreach::Foreach(libport::serialize::ISerializer<T>& ser)
    : Flavored(ser)
  {
    LIBPORT_USE(ser);
    index_ = ser.template unserialize< rLocalDeclaration >("index");
    list_ = ser.template unserialize< rExp >("list");
    body_ = ser.template unserialize< rScope >("body");
  }
#endif

  inline const rLocalDeclaration&
  Foreach::index_get () const
  {
    return index_;
  }
  inline rLocalDeclaration&
  Foreach::index_get ()
  {
    return index_;
  }

  inline const rExp&
  Foreach::list_get () const
  {
    return list_;
  }
  inline rExp&
  Foreach::list_get ()
  {
    return list_;
  }

  inline const rScope&
  Foreach::body_get () const
  {
    return body_;
  }
  inline rScope&
  Foreach::body_get ()
  {
    return body_;
  }


} // namespace ast

#endif // !AST_FOREACH_HXX
