/**
 ** \file object/object.cc
 ** \brief Implementation of object::Object.
 */

#include <algorithm>

#include <boost/assign.hpp> // for 'list_of'
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>
using namespace boost::assign; // bring 'list_of' into scope

#include <libport/containers.hh>
#include <libport/foreach.hh>

#include <object/atom.hh>
#include <object/dictionary-class.hh>
#include <object/float-class.hh>
#include <object/global-class.hh>
#include <object/hash-slots.hh>
#include <object/list-class.hh>
#include <object/object.hh>
#include <object/object-class.hh>
#include <object/urbi-exception.hh>

#include <runner/runner.hh>

namespace object
{

  /*---------.
  | Callers. |
  `---------*/

  rObject urbi_call(runner::Runner& r,
		    rObject self,
		    libport::Symbol msg,
		    objects_type args)
  {
    return urbi_call_function(r, self, self, msg, args);
  }

#define CHECK_ARG(N)				\
  if (!arg ## N)				\
    goto done;					\
  args.push_back(arg ## N)

  rObject urbi_call(runner::Runner& r,
		    rObject self,
		    libport::Symbol msg,
		    rObject arg1,
		    rObject arg2,
		    rObject arg3,
		    rObject arg4,
		    rObject arg5)
  {
    objects_type args;
    CHECK_ARG(1);
    CHECK_ARG(2);
    CHECK_ARG(3);
    CHECK_ARG(4);
    CHECK_ARG(5);
  done:
    return urbi_call_function(r, self, self, msg, args);
  }

#undef CHECK_ARG

  rObject urbi_call_function(runner::Runner& r, rObject self,
                             rObject owner, libport::Symbol msg)
  {
    objects_type args; // Call with no args.
    return urbi_call_function(r, self, owner, msg, args);
  }

  rObject urbi_call_function(runner::Runner& r, rObject self,
                             rObject owner, libport::Symbol msg,
                             objects_type args)
  {
    assertion(self);
    rObject message = owner->slot_get(msg);
    if (!message)
      throw LookupError(msg);
    args.insert(args.begin(), self);
    rObject res = r.apply(message, msg, args);
    assertion(res);
    return res;
  }

  /*---------.
  | Scopes.  |
  `---------*/

  typedef boost::optional<rObject> lookup_result;
  typedef boost::function1<lookup_result, rObject> lookup_action;

  rObject
  Object::make_scope(const rObject& parent)
  {
    rObject res = new object::Object();
    res->proto_add(parent);
    return res;
  }

  rObject
  Object::make_method_scope(const rObject& parent)
  {
    rObject res = Object::make_scope(parent ? parent : object_class);
    res->slot_set(SYMBOL(self), this);
    return res;
  }

  /*--------.
  | Slots.  |
  `--------*/

  rObject
  Object::urbi_protos_get ()
  {
    if (!protos_cache_)
    {
      rList protos = new List(*protos_);
      protos_cache_ = protos;
      delete protos_;
      protos_ = &protos->value_get ();
    }
    return protos_cache_;
  }

  void
  Object::protos_set (rObject l)
  {
    if (!protos_cache_)
      delete protos_;
    protos_cache_ = l;
    protos_ = &l.unsafe_cast<object::List>()->value_get ();
  }

  template <typename R>
  boost::optional<R>
  Object::lookup(boost::function1<boost::optional<R>, rObject> action,
		 objects_set_type& marks) const
  {
    if (!libport::mhas(marks, this))
    {
      marks.insert(this);
      // FIXME: rConstObject?
      boost::optional<R> res = action(const_cast<Object*>(this));
      if (res)
	return res;
      else
	foreach (const rObject& proto, protos_get())
	  if (boost::optional<R> res = proto->lookup(action, marks))
	    return res;
    }
    return boost::optional<R>();
  }

  template <typename R>
  boost::optional<R>
  Object::lookup(boost::function1<boost::optional<R>, rObject> action) const
  {
    objects_set_type marks;
    return lookup(action, marks);
  }

  namespace
  {
    class SlotLookup
    {
    public:
      lookup_result
      slot_lookup(rObject obj, const Object::key_type& k, bool value)
      {
	assertion(obj);
	if (rObject x = obj->own_slot_get(k))
	  return value ? x : obj;
	if (!fallback)
          if (rObject f = obj->own_slot_get(SYMBOL(fallback)))
            fallback = value ? f : obj;
	return boost::optional<rObject>();
      }
      rObject fallback;
    };
  }

  rObject
  Object::slot_locate(const key_type& k,
                      bool fallback, bool value) const
  {
    SlotLookup looker;
    lookup_action action =
      boost::bind(&SlotLookup::slot_lookup, &looker, _1, k, value);
    boost::optional<rObject> res = lookup(action);
    if (!res && fallback && looker.fallback)
      res = looker.fallback;
    if (res)
      return res.get();
    else
      return 0;
  }

  rObject
  Object::safe_slot_locate(const key_type& k, bool value) const
  {
    rObject r = slot_locate(k, true, value);
    if (!r)
      throw LookupError(k);
    return iassertion(r);
  }

  rObject
  Object::slot_get (const key_type& k, boost::optional<rObject> def) const
  {
    rObject value;
    if (def)
      value = slot_locate(k, true, true);
    else
      value = safe_slot_locate(k, true);
    if (value)
      return value;
    else
      return def.get();
  }


  Object&
  Object::slot_set(const key_type& k, rObject o)
  {
    if (!slots_.set(k, o))
      throw RedefinitionError(k);
    return *this;
  }

  Object&
  Object::slot_copy(const key_type& name, rObject from)
  {
    this->slot_set(name, from->slot_get(name));
    return *this;
  }

  void
  Object::slot_update(runner::Runner& r,
                      const key_type& k, rObject o,
                      bool hook)
  {
    // The owner of the updated slot
    rObject owner = safe_slot_locate(k);

    // If the current value in the slot to be written in has a slot
    // named 'updateHook', call it, passing the object owning the
    // slot, the slot name and the target.
    if (hook)
      if (rObject hook = owner->property_get(k, SYMBOL(updateHook)))
      {
        objects_type args = list_of (rObject(this)) (new String(k)) (o);
        rObject ret = r.apply(hook, SYMBOL(updateHook), args);
        // If the updateHook returned void, do nothing. Otherwise let
        // the slot be overwritten.
        if (ret == object::void_class)
          return;
      }
    // If return-value of hook is not void, write it to slot.
    own_slot_update(k, o);
  };

  void
  Object::own_slot_update (const key_type& k, rObject v)
  {
    slots_.update(k, v);
  }

  void
  Object::all_slots_copy(const rObject& other)
  {
    foreach (const Slots::slot_type& slot, other->slots_get())
      if (!own_slot_get(slot.first))
        slot_set(slot.first, slot.second);
  }


  /*-------------.
  | Properties.  |
  `-------------*/

  rDictionary
  Object::properties_get()
  {
    rDictionary res;
    if (slots_.has(SYMBOL(properties)))
      res = slots_.get(SYMBOL(properties)).unsafe_cast<Dictionary>();
    return res;
  }

  rDictionary
  Object::properties_get(const key_type& k)
  {
    rDictionary res;
    if (rDictionary ps = properties_get())
      res = libport::find0(ps->value_get(), k).unsafe_cast<Dictionary>();
    return res;
  }

  rObject
  Object::property_get(const key_type& k, const key_type& p)
  {
    rObject res;
    if (rDictionary ps = properties_get(k))
      res = libport::find0(ps->value_get(), p);
    return res;
  }

  void
  Object::property_set(const key_type& k, const key_type& p, rObject value)
  {
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

    prop->value_get()[p] = value;
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
    type_check<String>(data, SYMBOL(id_dump));
    libport::Symbol s = data->as<String>()->value_get();
    return o << s;
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
    protos_dump(o, runner);
    special_slots_dump (o, runner);
    foreach(const Slots::slot_type& s, slots_.container())
    {
      o << s.first << " = ";
      s.second->dump(o, runner, depth_max) << libport::iendl;
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
      type_check<String>(s, SYMBOL(print));
      o << s->as<String>()->value_get();
      return o;
    }
    // Check if asString was found, especially for bootstrap: asString
    // is implemented in urbi/urbi.u, but print is called to show
    // result in the toplevel before its definition.
    catch (LookupError&)
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
  is_true(const rObject& o, const libport::Symbol& fun)
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
      throw WrongArgumentType(fun.name_get());
    // FIXME: We will probably want to throw here.  This is related to
    // maybe calling asBool on every tested value.
    return true;
    // throw WrongArgumentType("Boolean", "Object", fun.name_get());
  }

} // namespace object
