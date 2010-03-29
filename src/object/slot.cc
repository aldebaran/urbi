/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
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
#include <object/symbols.hh>

namespace urbi
{
  namespace object
  {
    rObject
    Slot::property_get(libport::Symbol k)
    {
      if (k == SYMBOL(changed))
      {
        if (!changed_)
        {
          CAPTURE_GLOBAL(Event);
          changed_ = Event->call(SYMBOL(new));
        }
        return changed_;
      }
      if (properties_)
        return libport::find0(*properties_, k);
      return 0;
    }

    /// FIXME: does not work with "changed".
    bool
    Slot::property_has(libport::Symbol k) const
    {
      return properties_ && libport::mhas(*properties_, k);
    }

    bool
    Slot::property_set(libport::Symbol k, rObject value)
    {
      bool res = true;
      if (!properties_)
        properties_ = new properties_type;
      else
        res = !property_has(k);
      if (k == SYMBOL(constant))
        constant_ = from_urbi<bool>(value);
      (*properties_)[k] = value;
      return res;
    }

    void
    Slot::property_remove(libport::Symbol k)
    {
      if (properties_)
        properties_->erase(k);
    }

    // FIXME: Does not work with changed.
    Slot::properties_type*
    Slot::properties_get()
    {
      return properties_;
    }

    void
    Slot::constant_set(bool c)
    {
      constant_ = c;
      property_set(SYMBOL(constant), c ? true_class : false_class);
    }

    bool
    Slot::constant_get() const
    {
      return constant_;
    }
  }
}
