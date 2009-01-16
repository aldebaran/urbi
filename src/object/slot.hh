#ifndef OBJECT_SLOT_HH
# define OBJECT_SLOT_HH

# include <libport/assert.hh>
# include <libport/hash.hh>
# include <libport/intrusive-ptr.hh>
# include <libport/ref-counted.hh>
# include <libport/symbol.hh>

# include <object/fwd.hh>

namespace object
{
  class Slot: public libport::RefCounted
  {
  public:
    typedef libport::hash_map<libport::Symbol, rObject> properties_type;

    Slot();
    Slot(const Slot& model);
    template <typename T>
    Slot(const T& value);
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

    /*-----------.
    | Properties |
    `-----------*/

    rObject
    property_get(libport::Symbol k);
    bool
    property_has(libport::Symbol k);
    bool
    property_set(libport::Symbol k, rObject value);
    void
    property_remove(libport::Symbol k);
    properties_type&
    properties_get();

  private:
    rObject value_;
    rObject changed_;
    properties_type properties_;
  };

  typedef libport::intrusive_ptr<Slot> rSlot;
}

#endif
