/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/object/global.hh>
#include <urbi/object/slot.hh>
#include <urbi/object/slot.hxx>
# include <urbi/object/event.hh>
#include <object/symbols.hh>


// Help symbol generation script.
/* SYMBOL(value) */

namespace urbi
{
  namespace object
  {
    URBI_CXX_OBJECT_INIT(Slot)
    : constant_(false)
    , copyOnWrite_(true)
    , value_(void_class)
    {
      Ward w(this);
      // The BIND below will create slots that will use this, aka proto,
      // as their proto, so we must be valid right now.
      proto = this;
      proto_add(Object::proto);
      // FIXME: bind get/set mechanism in urbiscript
      BIND(value_, value_);
      BIND(set, set_);
      BIND(get, get_);
      BIND(oset, oset_);
      BIND(oget, oget_);
      BIND(constant, constant_);

      BIND(get_get); // debug
      BIND(set_get);
      BIND(oget_get); // debug
      BIND(oset_get);
      BIND(updateHook, updateHook_);
      BIND(copyOnWrite, copyOnWrite_);
      BIND(init);
      rSlot s(new Slot);
      slot_set(SYMBOL(changed), s);
      boost::function2<rObject, Slot&, rObject>
        getter(boost::bind(&Slot::changed, _1));
      s->oget_set(primitive(getter));
      s->constant_set(true);
    }
    rObject
    Slot::property_get(libport::Symbol k)
    {
      return slot_get_value(k, false);
    }

    /// FIXME: does not work with "changed".
    bool
    Slot::property_has(libport::Symbol k) const
    {
      return slot_has(k);
    }

    bool
    Slot::property_set(libport::Symbol k, rObject value)
    {
      Object::location_type r = slot_locate(k, true);
      if (!r.first)
        slot_set_value(k, value);
      else
        slot_update(k, value);
      return !r.first;
    }

    void
    Slot::property_remove(libport::Symbol k)
    {
      slot_remove(k);
    }

    // FIXME: Does not work with changed.
    Slot::properties_type*
    Slot::properties_get()
    {
      return 0;//local_slots_get();
    }

    void
    Slot::set(rObject value, Object* sender)
    {
      GD_CATEGORY(Urbi.Slot);
      GD_FINFO_DUMP("Slot::set, slot %s, sender %s, oset %s",
        this, sender, !!oset_);
      if (!sender)
      {
        GD_INFO_TRACE("Set without seneder");
      }
      if (constant_)
        runner::raise_const_error();
      if (sender && oset_)
      {
         object::objects_type args;
         args << value << this;
         eval::call_apply(::kernel::runner(),
                          sender, oset_, SYMBOL(oset), args);
      }
      if (set_)
      {
        object::objects_type args;
        args << value;
        eval::call_apply(::kernel::runner(),
                         const_cast<Slot*>(this), set_, SYMBOL(set), args);
      }
      else
      {
        value_ = value;
        assert(value_);
        static libport::Symbol _emit("emit");
        if (changed_)
          changed_->call(_emit);
      }
    }

    rObject
    Slot::init()
    {
      rSlot model = protos_get_first()->as<Slot>();
       if (model->set_)
        set_ = model->set_->call(SYMBOL(new));
      if (model->get_)
        get_ = model->get_->call(SYMBOL(new));
      if (model->oset_)
        oset_ = model->oset_->call(SYMBOL(new));
      if (model->oget_)
        oget_ = model->oget_->call(SYMBOL(new));
      return void_class;
    }

    Slot::Slot(rSlot model)
      : constant_(model->constant_)
      , copyOnWrite_(model->copyOnWrite_)
      , value_(model->value_)
    {
      Ward w(this);
      aver(model);
      proto_set(model);
      init();
    }

    Slot::Slot(const Slot& model)
      : CxxObject()
      , constant_(model.constant_)
      , copyOnWrite_(model.copyOnWrite_)
      , value_(model.value_)
    {
      //std::cerr <<"slot copy " << &model <<" -> " << this
      //<< " oset " << model.oset_ << std::endl;
      Ward w(this);
      aver(&model);
      proto_set(rSlot(const_cast<Slot*>(&model)));
      init();
      //NM: I don't think calling the setter is a good idea here
      // Main usage for this function is the COW that will call
      // the setter immediately after. So calling it with an outdated value
      // seems bad.
    }
    const size_t Slot::allocator_static_max_size = sizeof(Slot);
  }
}
