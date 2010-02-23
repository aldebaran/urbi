/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_SLOT_HH
# define OBJECT_SLOT_HH

# include <libport/cassert>
# include <libport/hash.hh>
# include <libport/intrusive-ptr.hh>
# include <libport/ref-counted.hh>
# include <libport/symbol.hh>

# include <urbi/object/fwd.hh>

namespace urbi
{
  namespace object
  {
    class Slot: public libport::RefCounted
    {
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
      operator bool ();
      Object* operator->();
      const Object* operator->() const;
      rObject value() const;
      void constant_set(bool c);
      bool constant_get() const;

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

    private:
      rObject value_;
      rObject changed_;
      bool constant_;
      properties_type* properties_;
    };

    typedef libport::intrusive_ptr<Slot> rSlot;
  }
}

#endif
