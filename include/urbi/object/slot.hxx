/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_SLOT_HXX
# define OBJECT_SLOT_HXX

# include <urbi/object/cxx-conversions.hh>
# include <urbi/runner/raise.hh>

namespace urbi
{
  namespace object
  {
    inline
    Slot::Slot()
      : value_(object::void_class)
      , constant_(false)
      , properties_(0)
    {}

    inline
    Slot::Slot(const Slot& model)
      : libport::RefCounted()
      , value_(model.value_)
      , constant_(false)
      , properties_(model.properties_
                    ? new properties_type(*model.properties_) : 0)
    {}

    template <typename T>
    inline
    Slot::Slot(const T& value)
      : value_(0)
      , constant_(false)
      , properties_(0)
    {
      set(value);
    }

    template <typename T>
    inline T
    Slot::get()
    {
      return object::CxxConvert<T>::to(value_, 0);
    }

    template <typename T>
    inline void
    Slot::set(const T& value)
    {
      if (constant_)
        runner::raise_const_error();
      value_ = object::CxxConvert<T>::from(value);
      static libport::Symbol _emit("emit");
      if (changed_)
        changed_->call(_emit);
    }

    template <typename T>
    inline const T&
    Slot::operator=(const T& value)
    {
      set(value);
      return value;
    }

    inline
    Slot::operator rObject ()
    {
      return get<rObject>();
    }

    inline
    Slot::operator bool ()
    {
      return value_.operator bool();
    }

    inline
    Object*
    Slot::operator->()
    {
      return assert_exp(value_.get());
    }

    inline
    const Object*
    Slot::operator->() const
    {
      return assert_exp(value_.get());
    }

    inline
    rObject
    Slot::value() const
    {
      return value_;
    }

    inline
    Slot::~Slot()
    {
      delete properties_;
    }
  }
}

#endif
