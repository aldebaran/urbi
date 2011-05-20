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
    : value_(void_class)
    {
      // The BIND below will create slots that will use this, aka proto,
      // as their proto, so we must be valid right now.
      proto = this;
      proto_add(Object::proto);
      // FIXME: bind get/set mechanism in urbiscript
      BIND(value_, value_);
      BIND(set, set_);
      BIND(get, get_);
      BIND(oset, set_);
      BIND(oget, get_);
      BIND(constant, constant_);
      BIND(get_get); // debug
      BIND(set_get);
      BIND(oget_get); // debug
      BIND(oset_get);
      BIND(updateHook, updateHook_);
      slot_set(SYMBOL(changed), void_class);
      boost::function2<rObject, Slot&, rObject>
        getter(boost::bind(&Slot::changed, _1));
      slot_get(SYMBOL(changed)).oget_set(primitive(getter));
      slot_get(SYMBOL(changed)).constant_set(true);
    }
    rObject
    Slot::property_get(libport::Symbol k)
    {
      Object::location_type r = slot_locate(k, true);
      if (!r.first)
        return 0;
      else
        return r.second->value(this);
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
        slot_set(k, value);
      else
        r.second->set(value, this);
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

    const size_t Slot::allocator_static_max_size = sizeof(Slot);
  }
}
