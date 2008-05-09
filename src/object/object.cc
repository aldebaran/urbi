/**
 ** \file object/object.cc
 ** \brief Implementation of object::Object.
 */

#include <algorithm>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>

#include <libport/containers.hh>
#include <libport/foreach.hh>

#include "object/object.hh"
#include "object/atom.hh"
#include "object/global-class.hh"
#include "object/hash-slots.hh"
#include "object/urbi-exception.hh"

#include "runner/runner.hh"

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
    assertion(self);
    rObject message = self->slot_get(msg);
    if (!message)
      throw LookupError(msg);
    args.insert(args.begin(), self);
    rObject res = r.apply(message, msg, args);
    assertion(res);
    return res;
  }

  rObject urbi_call(runner::Runner& r,
		    rObject self,
		    libport::Symbol msg)
  {
    objects_type args; // Call with no args.
    return urbi_call(r, self, msg, args);
  }

  /*-------.
  | kind.  |
  `-------*/

  const char*
  Object::string_of (Object::kind_type k)
  {
    switch (k)
    {
#define CASE(What, Name) case kind_ ## What: return #Name; break;
      APPLY_ON_ALL_PRIMITIVES(CASE);
#undef CASE
    }
    pabort("unreachable");
  }

  /*---------.
  | Scopes.  |
  `---------*/

  // Typedef for lookups that return rObject
  typedef boost::optional<rObject> lookup_result;
  typedef boost::function1<lookup_result, rObject> lookup_action;

  namespace
  {
    using std::make_pair;
    using boost::bind;
    using boost::optional;

    static lookup_result
    targetLookup(rObject obj,
		 const object::Slots::key_type& slotName)
    {
      // First, check if the object has the slot locally. We do not
      // handle 'self' here, since we first want to see whether it has
      // been captured in the context.
      if (slotName != SYMBOL(self) && obj->own_slot_get(slotName, 0))
	// Return a nonempty optional containing an empty rObject, to
	// indicate to target that the lookup is successful, and the
	// target is the initial object.
	return optional<rObject>(rObject());
      // If this is a method outer scope, perform special lookup in
      // self and context.
      if (rObject self = obj->own_slot_get(SYMBOL(self), 0))
      {
	// FIXME: The 'code' slot is *always* set by the scoping
	// system, yet the user can still delete it. What kind of
	// error should we raise when this problem occurs? For now,
	// just ignore it:
	if (rObject code = obj->own_slot_get(SYMBOL(code), 0))
	  // Likewise.
	  if (rObject captured = code->own_slot_get(SYMBOL(capturedVars), 0))
	  {
	    if (captured == nil_class)
	      return target(code->own_slot_get(SYMBOL(context)), slotName);
	    else
	      foreach (const rObject& var, captured->value<object::List>())
		if (var->value<object::String>() == slotName)
		  return target(code->own_slot_get(SYMBOL(context)), slotName);
	  }
	// If we were looking for 'self' and it wasn't captured in the
	// context, we found it here.
	if (slotName == SYMBOL(self))
	  return optional<rObject>(rObject());
	// Check whether self has the slot. We do not use the
	// 'fallback' method here: only calls with explicit target are
	// subject to fallback.
	if (self->slot_locate(slotName, false))
	  return self;
      }
      return optional<rObject>();
    }
  }

  rObject
  target(rObject where, const libport::Symbol& name)
  {
    boost::function1<boost::optional<rObject>, rObject> lookup =
      boost::bind(targetLookup, _1, name);
    boost::optional<rObject> res = where->lookup(lookup);
    if (!res)
      throw object::LookupError(name);
    if (!res.get())
      return where;
    return res.get();
  }

  rObject
  Object::make_scope(const rObject& parent)
  {
    rObject res = object::Object::fresh();
    res->locals_set(true);
    res->proto_add(parent);
    return res;
  }

  namespace
  {
    // helper for make_method_scope and make_do_scope
    static rObject
    make_outer_scope(const rObject& parent, const rObject& self)
    {
      rObject res = Object::make_scope(parent);
      res->slot_copy(SYMBOL(getSlot), scope_class);
      res->slot_copy(SYMBOL(locateSlot), scope_class);
      res->slot_copy(SYMBOL(removeSlot), scope_class);
      res->slot_copy(SYMBOL(updateSlot), scope_class);
      res->slot_set(SYMBOL(self), self);
      // We really need to copy 'locals' in every scope, or else
      // Scope's methods will get completely fubared: 'locals' will be
      // found in 'self' and will thus not be the local scope!
      res->slot_copy(SYMBOL(locals), scope_class);
      return res;
    }
  }

  rObject
  Object::make_method_scope(const rObject& self)
  {
    rObject res = make_outer_scope(global_class, self);
    res->slot_copy(SYMBOL(setSlot), scope_class);
    return res;
  }

  rObject
  Object::make_do_scope(const rObject& parent, const rObject& self)
  {
    rObject res = make_outer_scope (parent ? parent : global_class, self);
    res->slot_set(SYMBOL(setSlot), scope_class->slot_get(SYMBOL(doSetSlot)));
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
      rList protos = List::fresh (*protos_);
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
      assertion(self());
      boost::optional<R> res = action(self());
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
      slot_lookup(rObject obj, const Slots::key_type& k, bool value)
      {
	assertion(obj);
	if (rObject x = obj->own_slot_get(k, 0))
	  return value ? x : obj;
	if (!fallback)
          if (rObject f = obj->own_slot_get(SYMBOL(fallback), 0))
            fallback = value ? f : obj;
	return boost::optional<rObject>();
      }
      rObject fallback;
    };
  }

  rObject Object::slot_locate(const Slots::key_type& k,
                              bool fallback, bool value) const
  {
    SlotLookup looker;
    lookup_action action = boost::bind(&SlotLookup::slot_lookup, &looker, _1, k, value);
    boost::optional<rObject> res = lookup(action);
    if (!res && fallback && looker.fallback)
      res = looker.fallback;
    return res ? res.get() : 0;
  }

  rObject
  Object::safe_slot_locate(const Slots::key_type& k, bool value) const
  {
    rObject r = slot_locate(k, true, value);
    if (!r)
      throw LookupError(k);
    return iassertion(r);
  }

  rObject
  Object::slot_get (const Slots::key_type& k, boost::optional<rObject> def) const
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
  Object::slot_set (const Slots::key_type& k, rObject o)
  {
    if (!slots_.set(k, o))
      throw RedefinitionError(k);
    return *this;
  }

  Object&
  Object::slot_copy (const Slots::key_type& name, rObject from)
  {
    this->slot_set(name, from->slot_get(name));
    return *this;
  }

  void
  Object::slot_update (runner::Runner& r,
		       const Slots::key_type& k,
		       rObject o,
		       bool hook)
  {
    // The target of updateSlot
    rObject context = self();
    // The owner of the updated slot
    rObject owner = context->safe_slot_locate(k);

    // We have to determine where the new value must be stored,
    // depending on whether the slot owner and the context are scopes.
    rObject effective_target;

    // If both are scopes, update the original scope.
    if (context->locals_ && owner->locals_get())
      effective_target = owner;
    else if (context->locals_ && !owner->locals_get())
    {
      // Local->class: copyonwrite to "self" after evaluating it.
      rObject self = urbi_call(r, context, SYMBOL(self));
      assertion(self);
      effective_target = self;
    }
    else // Class->class: copy on write.
      effective_target = context;
    // Check hook, only if we are not create-on-writing.
    /* If the current value in the slot to be written in has a slot named
     * 'updateHook', call it, passing the object owning the slot, the slot name
     * and the target.
     */
    if (hook && effective_target == owner)
    // FIXME: We probably want helper to access properties
    if (rObject properties = effective_target->slot_get(SYMBOL(properties), rObject()))
    if (rObject slotProperties = properties->slot_get(k, rObject()))
    if (rObject hook = slotProperties->slot_get(SYMBOL(updateHook), rObject()))
    {
      objects_type args;
      args.push_back(effective_target);
      args.push_back(String::fresh(k));
      args.push_back(o);
      rObject ret = r.apply(hook, SYMBOL(updateHook), args);
      // If the updateHook returned void, do nothing. Otherwise let
      // the slot be overwritten.
      if (ret == object::void_class)
	return;
    }
    // If return-value of hook is not void, write it to slot.
    effective_target->own_slot_update(k, o);
  };

  void
  Object::own_slot_update (const Slots::key_type& k, rObject v)
  {
    slots_.update(k, v);
  }

  rObject
  Object::own_slot_get (const Slots::key_type& k, rObject def)
  {
    rObject res = slots_.get(k);
    return res ? res : def;
  }

  void
  Object::all_slots_copy(const rObject& other)
  {
    foreach (Slots::slot_type slot, other->slots_get())
      if (!own_slot_get(slot.first, 0))
        slot_set(slot.first, slot.second);
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
    rObject data = urbi_call(r, self(), SYMBOL(id));
    std::string s = data->value<String>().name_get();
    return o << s;
  }


  std::ostream&
  Object::protos_dump(std::ostream& o, runner::Runner& runner) const
  {
    if (!protos_->empty())
    {
      o << "protos = ";
      bool tail = false;
      foreach (rObject p, *protos_)
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
  Object::dump (std::ostream& o, runner::Runner& runner) const
  {
    id_dump(o, runner);
    /// Use xalloc/iword to store our current depth within the stream object.
    static const long idx = o.xalloc();
    long& current_depth = o.iword(idx);

    // Stop recursion at depth_max.
    enum { depth_max = 3 };
    if (current_depth > depth_max)
      return o << " <...>";
    ++current_depth;
    o << " {" << libport::incendl;
    protos_dump(o, runner);
    special_slots_dump (o, runner);
    foreach(Slots::slot_type s, slots_.container())
    {
      o << s.first << " = ";
      s.second->dump(o, runner) << libport::iendl;
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
      rObject s = urbi_call(runner, self(), SYMBOL(asString));
      o << s->value<String>().name_get();
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
  is_true(const rObject& o)
  {
    // FIXME: should nil be true? It should probably be false.
    if (o == nil_class)
      return true;
    if (o->type_is<object::Float>())
      return o.unsafe_cast<object::Float>()->value_get();
    return true;
  }

} // namespace object
