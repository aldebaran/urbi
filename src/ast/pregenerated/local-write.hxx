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
 ** \file ast/local-write.hxx
 ** \brief Inline methods of ast::LocalWrite.
 */

#ifndef AST_LOCAL_WRITE_HXX
# define AST_LOCAL_WRITE_HXX

# include <ast/local-write.hh>

namespace ast
{

#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  LocalWrite::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< libport::Symbol >("what", what_);
    ser.template serialize< rExp >("value", value_);
    ser.template serialize< unsigned >("local_index", local_index_);
  }

  template <typename T>
  LocalWrite::LocalWrite(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    what_ = ser.template unserialize< libport::Symbol >("what");
    value_ = ser.template unserialize< rExp >("value");
    local_index_ = ser.template unserialize< unsigned >("local_index");
  }
#endif

  inline const libport::Symbol&
  LocalWrite::what_get () const
  {
    return what_;
  }
  inline libport::Symbol&
  LocalWrite::what_get ()
  {
    return what_;
  }
  inline void
  LocalWrite::what_set (const libport::Symbol& what)
  {
    what_ = what;
  }

  inline const rExp&
  LocalWrite::value_get () const
  {
    return value_;
  }
  inline rExp&
  LocalWrite::value_get ()
  {
    return value_;
  }
  inline void
  LocalWrite::value_set (const rExp& value)
  {
    value_ = value;
  }

  inline const unsigned&
  LocalWrite::local_index_get () const
  {
    return local_index_;
  }
  inline unsigned&
  LocalWrite::local_index_get ()
  {
    return local_index_;
  }
  inline void
  LocalWrite::local_index_set (unsigned local_index)
  {
    local_index_ = local_index;
  }


} // namespace ast

#endif // !AST_LOCAL_WRITE_HXX
