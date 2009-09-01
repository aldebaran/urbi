/**
 ** \file object/object.cc
 ** \brief Implementation of object::Object.
 */

#include <algorithm>
#include <climits>

#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>

#include <libport/containers.hh>
#include <libport/contract.hh>
#include <libport/foreach.hh>
#include <libport/lexical-cast.hh>

#include <kernel/userver.hh>

#include <object/cxx-conversions.hh>
#include <object/dictionary.hh>
#include <object/float.hh>
#include <object/global.hh>
#include <object/list.hh>
#include <object/object.hh>
#include <object/root-classes.hh>
#include <object/symbols.hh>
#include <object/urbi-exception.hh>

#include <runner/raise.hh>
#include <runner/runner.hh>

namespace object
{

  namespace
  {
    /// Use xalloc/iword to store our current depth within the stream object.
    static
    inline
    long&
    current_depth(std::ostream& o)
    {
      static const long idx = std::ios::xalloc();
      return o.iword(idx);
    }
  }


  Object::Object()
    : protos_(new protos_type)
    , slots_()
    , lookup_id_(INT_MAX)
  {
    root_classes_initialize();
  }

  Object::~Object ()
  {
    slots_.finalize(this);
    if (!protos_cache_)
      delete protos_;
  }

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

  static int lookup_id = 0;

  inline Object::location_type
  Object::slot_locate_(const key_type& k, bool fallback) const
  {
    if (lookup_id_ == lookup_id)
      return location_type(0, 0);
    lookup_id_ = lookup_id;
    if (rSlot slot = local_slot_get(k))
      return location_type(const_cast<Object*>(this), slot);
    foreach (const rObject& proto, protos_get())
    {
      location_type rec = proto->slot_locate_(k, fallback);
      if (rec.first)
        return rec;
    }
    if (fallback)
      if (rSlot slot = local_slot_get(SYMBOL(fallback)))
        return location_type(const_cast<Object*>(this), slot);
    return location_type(0, 0);
  }

  Object::location_type
  Object::slot_locate(const key_type& k,
                      bool fallback) const
  {
    ++lookup_id;
    return slot_locate_(k, fallback);
  }

  Object::location_type
  Object::safe_slot_locate(const key_type& k) const
  {
    location_type r = slot_locate(k, true);
    if (!r.first)
      runner::raise_lookup_error(k, const_cast<Object*>(this));
    return r;
  }

  rObject
  Object::slot_get(const key_type& k) const
  {
    return const_cast<Object*>(this)->slot_get(k);
  }

  Slot&
  Object::slot_get(const key_type& k)
  {
    return *safe_slot_locate(k).second;
  }


  Object&
  Object::slot_set(const key_type& k, rObject o, bool constant)
  {
    Slot* slot = new Slot(o);
    slot->constant_set(constant);
    return slot_set(k, slot);
  }

  Object&
  Object::slot_set(const key_type& k, Slot* o)
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
  Object::slot_has(const key_type& k) const
  {
    return slot_locate(k).first;
  }

  rObject
  Object::slot_update(const key_type& k, const rObject& o,
                      bool hook)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();

    // The updated slot
    Slot& s = slot_get(k);
    // Its owner
    rObject owner = slot_locate(k).first;
    // Value to write to the slot
    rObject v = o;

    // If the current value in the slot to be written in has a slot
    // named 'updateHook', call it, passing the object owning the
    // slot, the slot name and the target.
    if (hook)
      if (rObject hook = s.property_get(SYMBOL(updateHook)))
      {
        objects_type args;
        args.push_back(rObject(this));
        args.push_back(new String(k));
        args.push_back(o);
        v = r.apply(hook, SYMBOL(updateHook), args);
        // If the updateHook returned void, do nothing. Otherwise let
        // the slot be overwritten.
        if (v == object::void_class)
          return o;
      }
    // If return-value of hook is not void, write it to slot.
    if (owner == this)
      s = v;
    else
    {
      // Here comes the cow
      Slot* slot = new Slot(s);
      *slot = v;
      slot_set(k, slot);
    }
    return v;
  };


  /*-------------.
  | Properties.  |
  `-------------*/

  rDictionary
  Object::properties_get()
  {
    Dictionary::value_type res;
    for (slots_implem::const_iterator slot = slots_.begin(this);
         slot != slots_.end(this);
         ++slot)
      res[slot->first.second] = properties_get(slot->first.second);
    return new Dictionary(res);
  }

  rDictionary
  Object::properties_get(const key_type& k)
  {
    if (Slot::properties_type* props = slot_get(k).properties_get())
      return new Dictionary(*props);
    else
      return new Dictionary();
  }

  rObject
  Object::property_get(const key_type& k, const key_type& p)
  {
    return slot_get(k).property_get(p);
  }

  bool
  Object::property_has(const key_type& k, const key_type& p)
  {
    return slot_get(k).property_has(p);
  }

  rObject
  Object::property_set(const key_type& k,
		       const key_type& p,
		       const rObject& value)
  {
    // CoW
    if (safe_slot_locate(k).first != this)
      slot_set(k, slot_get(k));
    Slot& slot = slot_get(k);
    if (slot.property_set(p, value))
      if (slot->slot_has(SYMBOL(newPropertyHook)))
        slot->call(SYMBOL(newPropertyHook),
                   this, new String(k), new String(p), value);
    return value;
  }

  rObject
  Object::property_remove(const key_type& k, const key_type& p)
  {
    Slot& slot = slot_get(k);
    rObject res = slot.property_get(p);
    slot.property_remove(p);
    return res;
  }

  /*-----------.
  | Printing.  |
  `-----------*/

  std::ostream&
  Object::special_slots_dump(std::ostream& o) const
  {
    return o;
  }

  std::ostream&
  Object::slot_dump(std::ostream& o,
                    const CentralizedSlots::q_slot_type& slot,
                    int depth_max) const
  {
    o << slot.first.second << " = ";
    slot.second->value()->dump(o, depth_max) << libport::iendl;
    if (Slot::properties_type* props = slot.second->properties_get())
    {
      o << "  /* Properties */" << libport::incendl;
      foreach (const Slot::properties_type::value_type& p, *props)
      {
        o << p.first << " = ";
        p.second->dump(o, depth_max);
      }
      o << libport::decendl;
    }
    return o;
  }

  bool
  Object::operator< (const Object& rhs) const
  {
    return this < &rhs;
  }

  std::ostream&
  Object::id_dump(std::ostream& o) const
  {
    rObject data = const_cast<Object*>(this)->call(SYMBOL(id));
    type_check<String>(data);
    return o << data->as<String>()->value_get();
  }


  std::ostream&
  Object::protos_dump(std::ostream& o) const
  {
    if (!protos_->empty())
    {
      o << "protos = ";
      bool tail = false;
      foreach (const rObject& p, *protos_)
      {
	if (tail++)
	  o << ", ";
	p->id_dump (o);
      }
      o << libport::iendl;
    }
    return o;
  }

  std::ostream&
  Object::dump(std::ostream& o, int depth_max) const
  {
    id_dump(o);

    // Stop recursion at depth_max.
    if (depth_max < current_depth(o))
      return o << " <...>";
    ++current_depth(o);

    o << " {" << libport::incendl
      << "/* Special slots */" << libport::iendl;
    protos_dump(o);
    special_slots_dump (o);

    o << "/* Slots */" << libport::iendl;
    for (slots_implem::const_iterator i = slots_.begin(this);
         i != slots_.end(this);
         ++i)
      slot_dump(o, *i, depth_max);

    o << libport::decindent << '}';
    --current_depth(o);
    return o;
  }

  std::ostream&
  Object::print(std::ostream& o) const
  {
    try
    {
      rObject s = const_cast<Object*>(this)->call(SYMBOL(asString));
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
  Object::as_bool() const
  {
    if (this == nil_class.get()
        || this == false_class.get())
      return false;

    if (this == void_class.get())
      runner::raise_unexpected_void_error();

    return true;
  }

  bool Object::valid_proto(const Object&) const
  {
    return true;
  }

  Object&
  Object::proto_add(const rObject& p)
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
      push_front (*protos_, p);
    return *this;
  }

  std::string
  Object::type_name_get() const
  {
    return "Object";
  }

  /*---------------.
  | Urbi methods.  |
  `---------------*/

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
    rObject o = slot_locate(name).first;
    return o ? o : nil_class;
  }

  rDictionary
  Object::urbi_properties(key_type name)
  {
    return properties_get(name);
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
  Object::urbi_setConstSlot(key_type name, const rObject& value)
  {
    slot_set(name, value, true);
    return value;
  }

  rObject
  Object::urbi_updateSlot(key_type name, const rObject& value)
  {
    return slot_update(name, value);
  }


#define CHECK_ARG(N)				\
  if (!arg ## N)				\
    goto done;					\
  args.push_back(arg ## N)

  rObject
  Object::call(libport::Symbol name,
               rObject arg1,
               rObject arg2,
               rObject arg3,
               rObject arg4,
               rObject arg5)
  {
    objects_type args;
    args.push_back(this);
    CHECK_ARG(1);
    CHECK_ARG(2);
    CHECK_ARG(3);
    CHECK_ARG(4);
    CHECK_ARG(5);
    done:

    return call(name, args);
  }

#undef CHECK_ARG

  rObject
  Object::call(libport::Symbol name,
               const objects_type& args)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    rObject fun = slot_get(name);
    return r.apply(fun, name, args);
  }

  rObject
  Object::call(const std::string& name,
               rObject arg1,
               rObject arg2,
               rObject arg3,
               rObject arg4,
               rObject arg5)
  {
    return call(libport::Symbol(name), arg1, arg2, arg3, arg4, arg5);
  }

  std::ostream& operator<< (std::ostream& s, Object& o)
  {
    s << o.call(SYMBOL(asString))->as<String>()->value_get();
    return s;
  }

  rObject Object::proto;

} // namespace object
