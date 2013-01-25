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
 ** \file ast/dictionary.hxx
 ** \brief Inline methods of ast::Dictionary.
 */

#ifndef AST_DICTIONARY_HXX
# define AST_DICTIONARY_HXX

# include <ast/dictionary.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Dictionary::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< dictionary_elts_type >("value", value_);
  }

  template <typename T>
  Dictionary::Dictionary(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    value_ = ser.template unserialize< dictionary_elts_type >("value");
  }
#endif

  inline const dictionary_elts_type&
  Dictionary::value_get () const
  {
    return value_;
  }
  inline dictionary_elts_type&
  Dictionary::value_get ()
  {
    return value_;
  }


} // namespace ast

#endif // !AST_DICTIONARY_HXX
