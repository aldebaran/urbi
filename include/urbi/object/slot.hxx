/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_SLOT_HXX
# define OBJECT_SLOT_HXX

# include <urbi/object/slot.hh>
# include <urbi/object/cxx-conversions.hh>
# include <urbi/runner/raise.hh>

/* Header below required to inline Slot::value(), but not part
 * of exported headers.
 * Do not touch the comment on the include line.
 */
#ifdef BUILDING_URBI_SDK
# include <runner/job.hh> // maintainer-check-includes-safe
#endif

namespace urbi
{
  namespace object
  {
    inline
    Slot::Slot()
      : constant_(false)
      , copyOnWrite_(true)
      , has_uvalue_(false)
      , value_(object::void_class)
    {
      Ward w(this);
      if (!proto)
        proto = new Slot(FirstPrototypeFlag());
      proto_add(proto);
      init();
    }

    template <typename T>
    inline rSlot
    Slot::create(const T& value)
    {
      return new Slot(object::CxxConvert<T>::from(value));
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
    bool
    Slot::constant() const
    {
      return constant_;
    }

/* This function is inlined for performances, but requires internal
 * headers.
 */
#ifdef BUILDING_URBI_SDK
    inline
    rObject
    Slot::value(Object* sender, bool fromUObject) const
    {
      if (runner::Job* r =
        ::kernel::urbiserver->getCurrentRunnerOpt())
      {
        if (r->dependencies_log_get())
          r->dependency_add(const_cast<Slot*>(this)->changed());
      }
       //URBI_AT_HOOK_(const_cast<Slot*>(this)->changed);
       if ( (sender && oget_) || get_ || has_uvalue_)
         return value_special(sender, fromUObject);
       else
         return split_ ? output_value_ : value_;
    }
#endif
  }
}

#endif
