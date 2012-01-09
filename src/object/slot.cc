/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
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
#include <urbi/object/symbols.hh>

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
#define URBI_OBJECT_SLOT_CACHED_PROPERTY_GET(R, Data, Elem)     \
      if (k == SYMBOL_EXPAND(BOOST_PP_TUPLE_ELEM(2, 1, Elem)))  \
        return to_urbi                                          \
          (BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 1, Elem), _));

      BOOST_PP_SEQ_FOR_EACH(URBI_OBJECT_SLOT_CACHED_PROPERTY_GET,
                            _, URBI_OBJECT_SLOT_CACHED_PROPERTIES);
#undef URBI_OBJECT_SLOT_CACHED_PROPERTY_GET
      if (properties_)
        return libport::find0(*properties_, k);
      return 0;
    }

    /// FIXME: does not work with "changed".
    bool
    Slot::property_has(libport::Symbol k) const
    {
#define URBI_OBJECT_SLOT_CACHED_PROPERTY_HAS(R, Data, Elem)     \
      if (k == SYMBOL_EXPAND(BOOST_PP_TUPLE_ELEM(2, 1, Elem)))  \
        return true;

      BOOST_PP_SEQ_FOR_EACH(URBI_OBJECT_SLOT_CACHED_PROPERTY_HAS,
                            _, URBI_OBJECT_SLOT_CACHED_PROPERTIES);
#undef URBI_OBJECT_SLOT_CACHED_PROPERTY_HAS
      return properties_ && libport::mhas(*properties_, k);
    }

    bool
    Slot::property_set(libport::Symbol k, rObject value)
    {
#define URBI_OBJECT_SLOT_CACHED_PROPERTY_SET(R, Data, Elem)     \
      if (k == SYMBOL_EXPAND(BOOST_PP_TUPLE_ELEM(2, 1, Elem)))  \
      {                                                         \
        BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 1, Elem), _set)     \
          (from_urbi<BOOST_PP_TUPLE_ELEM(2, 0, Elem)>(value));  \
        return false;                                           \
      }

      BOOST_PP_SEQ_FOR_EACH(URBI_OBJECT_SLOT_CACHED_PROPERTY_SET,
                            _, URBI_OBJECT_SLOT_CACHED_PROPERTIES);
#undef URBI_OBJECT_SLOT_CACHED_PROPERTY_SET

      properties_type::iterator i = properties_get()->find(k);
      if (i == properties_->end())
      {
        (*properties_)[k] = value;
        return true;
      }
      else
      {
        i->second = value;
        return false;
      }
    }

    void
    Slot::property_remove(libport::Symbol k)
    {
      // FIXME: Err for cached properties?
      if (properties_)
        properties_->erase(k);
    }

    // FIXME: Does not work with changed.
    Slot::properties_type*
    Slot::properties_get()
    {
      if (!properties_)
      {
        properties_ = new properties_type;
#define URBI_OBJECT_SLOT_CACHED_PROPERTY_SET(R, Data, Elem)             \
        (*properties_)[SYMBOL_EXPAND(BOOST_PP_TUPLE_ELEM(2, 1, Elem))] = \
          to_urbi(BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 1, Elem), _));

      BOOST_PP_SEQ_FOR_EACH(URBI_OBJECT_SLOT_CACHED_PROPERTY_SET,
                            _, URBI_OBJECT_SLOT_CACHED_PROPERTIES);
#undef URBI_OBJECT_SLOT_CACHED_PROPERTY_SET
      }
      return properties_;
    }

    const size_t Slot::allocator_static_max_size = sizeof(Slot);
  }
}
