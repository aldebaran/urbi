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
 ** \file ast/at.hxx
 ** \brief Inline methods of ast::At.
 */

#ifndef AST_AT_HXX
# define AST_AT_HXX

# include <ast/at.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  At::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Flavored::serialize(ser);
    ser.template serialize< bool >("sync", sync_);
    ser.template serialize< rExp >("cond", cond_);
    ser.template serialize< rExp >("body", body_);
    ser.template serialize< rExp >("onleave", onleave_);
    ser.template serialize< rExp >("duration", duration_);
  }

  template <typename T>
  At::At(libport::serialize::ISerializer<T>& ser)
    : Flavored(ser)
  {
    LIBPORT_USE(ser);
    sync_ = ser.template unserialize< bool >("sync");
    cond_ = ser.template unserialize< rExp >("cond");
    body_ = ser.template unserialize< rExp >("body");
    onleave_ = ser.template unserialize< rExp >("onleave");
    duration_ = ser.template unserialize< rExp >("duration");
  }
#endif

  inline const loc&
  At::flavor_location_get () const
  {
    return flavor_location_;
  }
  inline void
  At::flavor_location_set (const loc& flavor_location)
  {
    flavor_location_ = flavor_location;
  }

  inline const bool&
  At::sync_get () const
  {
    return sync_;
  }
  inline void
  At::sync_set (bool sync)
  {
    sync_ = sync;
  }

  inline const rExp&
  At::cond_get () const
  {
    return cond_;
  }
  inline rExp&
  At::cond_get ()
  {
    return cond_;
  }
  inline void
  At::cond_set (const rExp& cond)
  {
    cond_ = cond;
  }

  inline const rExp&
  At::body_get () const
  {
    return body_;
  }
  inline rExp&
  At::body_get ()
  {
    return body_;
  }
  inline void
  At::body_set (const rExp& body)
  {
    body_ = body;
  }

  inline const rExp&
  At::onleave_get () const
  {
    return onleave_;
  }
  inline rExp&
  At::onleave_get ()
  {
    return onleave_;
  }
  inline void
  At::onleave_set (rExp onleave)
  {
    onleave_ = onleave;
  }

  inline const rExp&
  At::duration_get () const
  {
    return duration_;
  }
  inline rExp&
  At::duration_get ()
  {
    return duration_;
  }
  inline void
  At::duration_set (rExp duration)
  {
    duration_ = duration;
  }


} // namespace ast

#endif // !AST_AT_HXX
