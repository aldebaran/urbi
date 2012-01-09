/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
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

# include <urbi/object/symbols.hh>
# include <urbi/object/fwd.hh>

# define URBI_OBJECT_MAX 1024 * 4

namespace urbi
{
  namespace object
  {
#define URBI_OBJECT_SLOT_CACHED_PROPERTIES      \
    ((bool, constant))                          \

    class Slot
      : public libport::RefCounted
      , public libport::StaticallyAllocated<Slot, URBI_OBJECT_MAX>
    {
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
      T get();
      template <typename T>
      void set(const T& value);
      template <typename T>
      const T& operator=(const T& value);
      operator rObject ();
      operator Object* ();
      operator bool ();
      Object* operator->();
      const Object* operator->() const;
      rObject value() const;

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

#define URBI_OBJECT_SLOT_CACHED_PROPERTY_DECLARE(R, Data, Elem)         \
        ATTRIBUTE_Rw(BOOST_PP_TUPLE_ELEM(2, 0, Elem),                   \
                     BOOST_PP_TUPLE_ELEM(2, 1, Elem))                   \
        {                                                               \
          if (BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 1, Elem), _) != val)  \
          {                                                             \
            BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 1, Elem), _) = val;     \
            if (properties_)                                            \
              (*properties_)[SYMBOL_EXPAND(BOOST_PP_TUPLE_ELEM(2, 1, Elem))]   \
                = to_urbi(val);                                         \
          }                                                             \
        }                                                               \

        BOOST_PP_SEQ_FOR_EACH(URBI_OBJECT_SLOT_CACHED_PROPERTY_DECLARE,
                              _, URBI_OBJECT_SLOT_CACHED_PROPERTIES);
#undef URBI_OBJECT_SLOT_CACHED_PROPERTY_DECLARE

    private:
      rObject value_;
      rObject changed_;
      properties_type* properties_;
    };

    typedef libport::intrusive_ptr<Slot> rSlot;
  }
}

#endif
