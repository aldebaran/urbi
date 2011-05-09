/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_SLOT_HXX
# define OBJECT_SLOT_HXX

# include <eval/fwd.hh>
# include <urbi/object/cxx-conversions.hh>
# include <urbi/runner/raise.hh>

namespace urbi
{
  namespace object
  {
    inline
    Slot::Slot()
      : constant_(false)
      , value_(object::void_class)
      , properties_(0)
    {
      proto_add(proto);
    }

    inline
    Slot::Slot(const Slot& model)
      : Object()
      , constant_(false)
      , value_(0)
      , properties_(model.properties_
                    ? new properties_type(*model.properties_) : 0)
    {
      proto_add(proto);
      set(model.value_);
    }

    template <typename T>
    inline
    Slot::Slot(const T& value)
      : constant_(false)
      , value_(0)
      , properties_(0)
    {
      proto_add(proto);
      set(value);
    }

    inline
    Slot::~Slot()
    {
      delete properties_;
    }

    template <typename T>
    inline T
    Slot::get()
    {
      return from_urbi<T>(value());
    }

    template <typename T>
    inline void
    Slot::set(const T& value)
    {
      if (constant_)
        runner::raise_const_error();
      if (set_)
      {
        object::objects_type args;
        args << object::CxxConvert<T>::from(value);
        eval::call_apply(::kernel::runner(),
                         const_cast<Slot*>(this), set_, SYMBOL(set), args);
      }
      else
        value_ = object::CxxConvert<T>::from(value);
      if (changed_)
        changed_->call(SYMBOL(emit));
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
      return value();
    }

    inline
    Object*
    Slot::operator->()
    {
      return assert_exp(value());
    }

    inline
    const Object*
    Slot::operator->() const
    {
      return assert_exp(value());
    }

    inline
    rObject
    Slot::value() const
    {
      if (get_)
        return eval::call_apply(::kernel::runner(),
                                const_cast<Slot*>(this),
                                get_, SYMBOL(get));
      else
        return value_;
    }

    inline
    bool
    Slot::constant() const
    {
      return constant_;
    }
  }
}

#endif
