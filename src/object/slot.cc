#include <object/global.hh>
#include <object/slot.hh>
#include <object/slot.hxx>
#include <object/symbols.hh>

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
    if (k == SYMBOL(constant))
      return to_urbi(constant_);
    properties_type::iterator it = properties_.find(k);
    if (it == properties_.end())
      return 0;
    else
      return it->second;
  }

  bool
  Slot::property_has(libport::Symbol k)
  {
    return properties_.find(k) != properties_.end();
  }

  bool
  Slot::property_set(libport::Symbol k, rObject value)
  {
    bool res = !property_has(k);
    if (k == SYMBOL(constant))
      constant_ = from_urbi<bool>(value);
    properties_[k] = value;
    return res;
  }

  void
  Slot::property_remove(libport::Symbol k)
  {
    properties_.erase(k);
  }

  Slot::properties_type&
  Slot::properties_get()
  {
    return properties_;
  }

  void
  Slot::constant_set(bool c)
  {
    constant_ = c;
  }

  bool
  Slot::constant_get() const
  {
    return constant_;
  }
}
