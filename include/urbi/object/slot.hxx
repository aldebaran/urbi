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
# include <urbi/object/slot.hh>
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
    {
      Ward w(this);
      if (!proto)
        proto = new Slot(FirstPrototypeFlag());
      proto_add(proto);
    }

    template <typename T>
    inline
    Slot::Slot(const T& value)
      : constant_(false)
      , value_(void_class)
    {
      Ward w(this);
      if (!proto)
        proto = new Slot(FirstPrototypeFlag());
      proto_add(proto);
      value_ = object::CxxConvert<T>::from(value);
    }

    inline
    Slot::~Slot()
    {
    }

    template <typename T>
    inline T
    Slot::get(Object* sender)
    {
      return from_urbi<T>(value(sender));
    }

    template <typename T>
    inline void
    Slot::set(const T& value, Object* sender)
    {
      set<rObject>(object::CxxConvert<T>::from(value), sender);
    }

    template <typename T>
    inline const T&
    Slot::operator=(const T& value)
    {
      set(value);
      return value;
    }

    inline
    rObject
    Slot::changed()
    {
      CAPTURE_GLOBAL(Event);
      if (!changed_)
        changed_ = Event->call(SYMBOL(new));
      return changed_;
    }

    /*
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
    */

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
    Slot::value(Object* sender) const
    {
      if (sender && oget_)
      {
         object::objects_type args;
         args << const_cast<Slot*>(this);
         return eval::call_apply(::kernel::runner(),
                                 sender,
                                 oget_, SYMBOL(oget), args);
      }
      if (get_)
      {
        object::objects_type args;
        return eval::call_apply(::kernel::runner(),
                              const_cast<Slot*>(this),
                              get_, SYMBOL(get), args);
      }
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
