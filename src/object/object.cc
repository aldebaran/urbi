/**
 ** \file object/object.cc
 ** \brief Implementation of object::Object.
 */

#include <algorithm>

#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>

#include <libport/containers.hh>
#include <libport/contract.hh>
#include <libport/foreach.hh>

#include <object/cxx-conversions.hh>
#include <object/dictionary.hh>
#include <object/float.hh>
#include <object/global.hh>
#include <object/hash-slots.hh>
#include <object/list.hh>
#include <object/object.hh>
#include <object/urbi-exception.hh>

#include <runner/call.hh>
#include <runner/raise.hh>
#include <runner/runner.hh>

namespace object
{

  /*--------.
  | Slots.  |
  `--------*/

  rList
  Object::urbi_protos_get()
  {
    if (protos_cache_)
      return protos_cache_->as<List>();

    rList protos = new List(*protos_);
    protos_cache_ = protos;
    delete protos_;
    protos_ = &protos->value_get();
    return protos;
  }

  void
  Object::protos_set(const rList& l)
  {
    if (!protos_cache_)
      delete protos_;
    protos_cache_ = l;
    protos_ = &l->value_get();
  }

  inline rObject
  Object::slot_locate(const key_type& k,
                      bool fallback, bool value, objects_set_type& marks) const
  {
    if (libport::mhas(marks, this))
      return 0;
    marks.insert(this);
    rObject res;
    if (res = own_slot_get(k))
      return value ? res : const_cast<Object*>(this);
    foreach (const rObject& proto, protos_get())
      if (rObject rec = proto->slot_locate(k, fallback, value, marks))
        return rec;
    if (fallback)
      return own_slot_get(SYMBOL(fallback));
    return 0;
  }

  rObject
  Object::slot_locate(const key_type& k,
                      bool fallback, bool value) const
  {
    objects_set_type marks;
    return slot_locate(k, fallback, value, marks);
  }

  rObject
  Object::safe_slot_locate(const key_type& k, bool value) const
  {
    rObject r = slot_locate(k, true, value);
    if (!r)
      runner::raise_lookup_error(k, const_cast<Object*>(this));
    return iassertion(r);
  }

  rObject
  Object::slot_get (const key_type& k) const
  {
    return safe_slot_locate(k, true);
  }


  Object&
  Object::slot_set(const key_type& k, const rObject& o)
  {
    if (!slots_.set(this, k, o))
      runner::raise_urbi_skip(SYMBOL(RedefinitionError), to_urbi(k));
    return *this;
  }

  Object&
  Object::slot_copy(const key_type& name, const rObject& from)
  {
    this->slot_set(name, from->slot_get(name));
    return *this;
  }

  bool
  Object::slot_has(const key_type& k)
  {
    return slot_locate(k);
  }

  rObject
  Object::slot_update(runner::Runner& r,
                      const key_type& k, const rObject& o,
                      bool hook)
  {
    // The owner of the updated slot
    rObject owner = safe_slot_locate(k);
    rObject v = o;

    // If the current value in the slot to be written in has a slot
    // named 'updateHook', call it, passing the object owning the
    // slot, the slot name and the target.
    if (hook)
      if (rObject hook = owner->property_get(k, SYMBOL(updateHook)))
      {
        objects_type args;
        args.push_back(new String(k));
        args.push_back(o);
        v = r.apply(rObject(this), hook, SYMBOL(updateHook), args);
        // If the updateHook returned void, do nothing. Otherwise let
        // the slot be overwritten.
        if (v == object::void_class)
          return o;
      }
    // If return-value of hook is not void, write it to slot.
    own_slot_update(k, v);
    return v;
  };

  void
  Object::own_slot_update (const key_type& k, const rObject& v)
  {
    slots_.update(this, k, v);
  }

  void
  Object::all_slots_copy(const rObject& other)
  {
    for (slots_implem::iterator slot = slots_.begin(other.get());
         slot != slots_.end(other.get());
         ++slot)
      if (!own_slot_get(slot->first.second))
        slot_set(slot->first.second, slot->second);
  }


  /*-------------.
  | Properties.  |
  `-------------*/

  rDictionary
  Object::properties_get()
  {
    if (slots_.has(this, SYMBOL(properties)))
      return slots_.get(this, SYMBOL(properties)).unsafe_cast<Dictionary>();
    return 0;
  }

  rDictionary
  Object::properties_get(const key_type& k)
  {
    // Forbid searching properties on nonexistent slots
    safe_slot_locate(k);

    if (rDictionary ps = properties_get())
      return libport::find0(ps->value_get(), k).unsafe_cast<Dictionary>();
    return 0;
  }

  rObject
  Object::property_get(const key_type& k, const key_type& p)
  {
    rObject owner = safe_slot_locate(k);

    if (rDictionary ps = owner->properties_get(k))
      return libport::find0(ps->value_get(), p);
    return 0;
  }

  bool
  Object::property_has(const key_type& k, const key_type& p)
  {
    // Look for properties in the owner of the slot
    rObject owner = safe_slot_locate(k);

    if (rDictionary ps = owner->properties_get(k))
      return libport::find0(ps->value_get(), p);
    return false;
  }

  rObject
  Object::property_set(runner::Runner& r,
                       const key_type& k,
		       const key_type& p,
		       const rObject& value)
  {
    // Forbid setting properties on nonexistent slots
    rObject owner = safe_slot_locate(k);
    // CoW
    if (owner != this)
      slot_set(k, slot_get(k));

    // Make sure the object has a properties dictionary.
    rDictionary props = properties_get();
    if (!props)
    {
      props = new Dictionary();
      // This should die if there is a slot name "properties" which is
      // not a dictionary, which is what we want, don't we?
      slot_set(SYMBOL(properties), props);
    }

    // Make sure we have a dict for slot k.
    rDictionary prop =
      libport::find0(props->value_get(), k).unsafe_cast<Dictionary>();
    if (!prop)
    {
      prop = new Dictionary();
      props->value_get()[k] = prop;
    }

    const bool had = prop->has(p);

    prop->value_get()[p] = value;

    if (!had)
    {
      rObject target = slot_get(k);
      if (target->slot_locate(SYMBOL(newPropertyHook)))
        urbi_call(r, target, SYMBOL(newPropertyHook),
                  this, new String(k), new String(p), value);
    }

    return value;
  }

  rObject
  Object::property_remove(const key_type& k, const key_type& p)
  {
     // Forbid searching properties on nonexistent slots
    safe_slot_locate(k);

    rObject res = void_class;

    if (rDictionary ps = properties_get(k))
    {
      Dictionary::value_type& dict = ps->value_get();
      const Dictionary::value_type::iterator i = dict.find(p);
      if (i != dict.end())
      {
	res = i->second;
	dict.erase(i);
      }
    }
    return res;
  }

  /*-----------.
  | Printing.  |
  `-----------*/

  std::ostream&
  Object::special_slots_dump (std::ostream& o, runner::Runner&) const
  {
    return o;
  }

  bool
  Object::operator< (const Object& rhs) const
  {
    return this < &rhs;
  }

  std::ostream&
  Object::id_dump(std::ostream& o, runner::Runner& r) const
  {
    rObject data = urbi_call(r, const_cast<Object*>(this), SYMBOL(id));
    type_check<String>(data);
    return o << data->as<String>()->value_get();
  }


  std::ostream&
  Object::protos_dump(std::ostream& o, runner::Runner& runner) const
  {
    if (!protos_->empty())
    {
      o << "protos = ";
      bool tail = false;
      foreach (const rObject& p, *protos_)
      {
	if (tail++)
	  o << ", ";
	p->id_dump (o, runner);
      }
      o << libport::iendl;
    }
    return o;
  }

  std::ostream&
  Object::dump (std::ostream& o, runner::Runner& runner, int depth_max) const
  {
    id_dump(o, runner);
    /// Use xalloc/iword to store our current depth within the stream object.
    static const long idx = o.xalloc();
    long& current_depth = o.iword(idx);

    // Stop recursion at depth_max.
    if (current_depth > depth_max)
      return o << " <...>";
    ++current_depth;
    o << " {" << libport::incendl;
    o << "/* Special slots */" << libport::iendl;
    protos_dump(o, runner);
    special_slots_dump (o, runner);

    o << "/* Slots */" << libport::iendl;
    for (slots_implem::const_iterator slot = slots_.begin(this);
         slot != slots_.end(this);
         ++slot)
    {
      o << slot->first.second << " = ";
      slot->second->dump(o, runner, depth_max) << libport::iendl;
    }

    o << libport::decindent << '}';
    //We can not reuse current_depth variable above according to spec.
    o.iword(idx)--;
    return o;
  }

  std::ostream&
  Object::print(std::ostream& o, runner::Runner& runner) const
  {
    try
    {
      rObject s = urbi_call(runner, const_cast<Object*>(this), SYMBOL(asString));
      type_check<String>(s);
      o << s->as<String>()->value_get();
      return o;
    }
    // Check if asString was found, especially for bootstrap: asString
    // is implemented in urbi/urbi.u, but print is called to show
    // result in the toplevel before its definition.
    catch (UrbiException&)
    {
      // If no asString method is supplied, print the unique id
      return o << std::hex << this;
    }
  }

  bool
  is_a(const rObject& c, const rObject& p)
  {
    return for_all_protos(c, boost::lambda::_1 == p);
  }

  bool
  is_true(const rObject& o)
  {
    if (o == nil_class)
      return false;
    if (o->is_a<object::Float>())
      return o.unsafe_cast<object::Float>()->value_get();
    if (o == true_class)
      return true;
    if (o == false_class)
      return false;
    if (o == void_class)
      runner::raise_unexpected_void_error();
    // FIXME: We will probably want to throw here.  This is related to
    // maybe calling asBool on every tested value.
    return true;
  }

  bool Object::valid_proto(const Object&) const
  {
    return true;
  }

  Object&
  Object::proto_add (const rObject& p)
  {
    assert(p);
    // Inheriting from atoms is a problem: we cannot morph in place
    // the C++ object to give him the right primitive type. For now,
    // we forbid inheriting from atoms.
    if (!p->valid_proto(*this))
    {
      boost::format fmt("cannot inherit from a %1% without being a %1% too");
      runner::raise_primitive_error((fmt % p->type_name_get()).str());
    }

    if (!libport::has(*protos_, p))
      protos_->push_front (p);
    return *this;
  }

  std::string
  Object::type_name_get() const
  {
    return "Object";
  }

  /*-------------.
  | Urbi methods |
  `-------------*/

  void
  Object::urbi_createSlot(key_type name)
  {
    slot_set(name, void_class);
  }

  rObject
  Object::urbi_getSlot(key_type name)
  {
    return slot_get(name);
  }

  rObject
  Object::urbi_locateSlot(key_type name)
  {
    rObject o = slot_locate(name);
    return o ? o : nil_class;
  }

  rObject
  Object::urbi_removeSlot(key_type name)
  {
    slot_remove(name);
    return this;
  }

  rObject
  Object::urbi_setSlot(key_type name, const rObject& value)
  {
    slot_set(name, value);
    return value;
  }

  rObject
  Object::urbi_updateSlot(runner::Runner& r,
                          key_type name, const rObject& value)
  {
    return slot_update(r, name, value);
  }

} // namespace object
