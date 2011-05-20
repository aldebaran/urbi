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
      if (!proto)
        proto = new Slot(FirstPrototypeFlag());
      proto_add(proto);
    }

    inline
    Slot::Slot(const Slot& model)
      : CxxObject()
      , constant_(model.constant_)
      , value_(model.value_)
    {
      aver(&model);
      proto_add(rSlot(const_cast<Slot*>(&model)));
      if (model.set_)
        set_ = model.set_->call(SYMBOL(new));
      if (model.get_)
        get_ = model.get_->call(SYMBOL(new));
      if (model.oset_)
        oset_ = model.oset_->call(SYMBOL(new));
      if (model.oget_)
        oget_ = model.oget_->call(SYMBOL(new));
      //NM: I don't think calling the setter is a good idea here
      // Main usage for this function is the COW that will call
      // the setter immediately after. So calling it with an outdated value
      // seems bad.
    }

    template <typename T>
    inline
    Slot::Slot(const T& value)
      : constant_(false)
      , value_(void_class)
    {
      if (!proto)
        proto = new Slot(FirstPrototypeFlag());
      proto_add(proto);
      set(value);
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
      if (constant_)
        runner::raise_const_error();
      if (sender && oset_)
      {
         object::objects_type args;
         args << object::CxxConvert<T>::from(value) << this;
         eval::call_apply(::kernel::runner(),
                          sender, oset_, SYMBOL(oset), args);
      }
      if (set_)
      {
        object::objects_type args;
        args << object::CxxConvert<T>::from(value);
        eval::call_apply(::kernel::runner(),
                         const_cast<Slot*>(this), set_, SYMBOL(set), args);
      }
      else
      {
        value_ = object::CxxConvert<T>::from(value);
        assert(value_);
      }
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
