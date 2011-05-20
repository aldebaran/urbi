/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_SLOT_HH
# define OBJECT_SLOT_HH

# include <boost/preprocessor/seq/for_each.hpp>
# include <boost/preprocessor/tuple/elem.hpp>


# include <libport/allocator-static.hh>
# include <libport/attributes.hh>
# include <libport/cassert>
# include <libport/hash.hh>
# include <libport/intrusive-ptr.hh>
# include <libport/ref-counted.hh>
# include <libport/symbol.hh>

# include <object/symbols.hh>
# include <urbi/object/fwd.hh>
# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
#define URBI_OBJECT_SLOT_CACHED_PROPERTIES      \
    ((bool, constant, 1))                       \
    ((rObject, get, 0))                         \
    ((rObject, set, 0))                         \
    ((rObject, value, 0))                       \

    class Slot: public CxxObject
    {
    private:
      URBI_CXX_OBJECT(Slot, CxxObject);
    public:
      /// Maximum object size for the allocator
      static const size_t allocator_static_max_size;

    public:
      typedef boost::unordered_map<libport::Symbol, rObject> properties_type;

      Slot();
      Slot(const Slot& model);
      template <typename T>
      Slot(const T& value);
      ~Slot();
      template <typename T>
      T get(Object* sender = 0);
      template <typename T>
      void set(const T& value, Object* sender=0);
      template <typename T>
      const T& operator=(const T& value);
      //operator rObject ();
      //operator bool ();
      Object* operator->();
      const Object* operator->() const;
      rObject value(Object* sender = 0) const;

      /*-------------.
      | Properties.  |
      `-------------*/

      /// The \a k property for this slot, or 0 if there is none.
      rObject property_get(libport::Symbol k);
      /// Whether this slot has a \a k property
      bool property_has(libport::Symbol k) const;
      /// Set the \a k property of this slot to \a value
      bool property_set(libport::Symbol k, rObject value);
      /// Remove the \a k property of this slot. Noop if it does not
      /// exist.
      void property_remove(libport::Symbol k);
      /// The properties hash map of this slot, or NULL if there are no
      /// property at all.
      properties_type* properties_get();
      /// Whether is const.
      bool constant() const;

#define URBI_OBJECT_SLOT_CACHED_PROPERTY_STORE(Elem)                    \
        (*properties_)                                                  \
        [SYMBOL_EXPAND(BOOST_PP_TUPLE_ELEM(3, 1, Elem))]                \
        = to_urbi(val)                                                  \

      rObject changed();
    protected:
      ATTRIBUTE_RW(rObject, changed);
      ATTRIBUTE_RW(bool, constant);
      // Slot getter hook: val slot.get()
      ATTRIBUTE_RW(rObject, get);
      // Slot setter hook: slot.set(val)
      ATTRIBUTE_RW(rObject, set);
      // Owner object getter hook: val obj.get(slot)
      ATTRIBUTE_RW(rObject, oget);
      // Owner object setter hook: obj.set(val, slot)
      ATTRIBUTE_RW(rObject, oset);
      ATTRIBUTE_RW(rObject, value);
      ATTRIBUTE_RW(rObject, updateHook);
    };

    typedef libport::intrusive_ptr<Slot> rSlot;
  }
}

# include <urbi/object/centralized-slots.hxx>
# include <urbi/object/cxx-object.hxx>
#endif
