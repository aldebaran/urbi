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
 ** \file ast/event.hxx
 ** \brief Inline methods of ast::Event.
 */

#ifndef AST_EVENT_HXX
# define AST_EVENT_HXX

# include <ast/event.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Event::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< rExp >("exp", exp_);
  }

  template <typename T>
  Event::Event(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    exp_ = ser.template unserialize< rExp >("exp");
  }
#endif

  inline const rExp&
  Event::exp_get () const
  {
    return exp_;
  }
  inline rExp&
  Event::exp_get ()
  {
    return exp_;
  }
  inline void
  Event::exp_set (const rExp& exp)
  {
    exp_ = exp;
  }


} // namespace ast

#endif // !AST_EVENT_HXX
