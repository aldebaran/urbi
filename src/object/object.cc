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
		 const object::Object::key_type& slotName)
    {
      if (obj->own_slot_get(slotName, 0) && slotName != SYMBOL(self))
	// Return a nonempty optional containing an empty rObject, to
	// indicate to target that the lookup is successful, and the
	// target is the initial object.
	return optional<rObject>(rObject());
      // If this is a method outer scope, perform special lookup
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
	if (slotName == SYMBOL(self))
	  return optional<rObject>(rObject());
	if (self->slot_locate(slotName))
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
      res->slot_set(SYMBOL(getSlot), scope_class->slot_get(SYMBOL(getSlot)));
      res->slot_set(SYMBOL(locateSlot), scope_class->slot_get(SYMBOL(locateSlot)));
      res->slot_set(SYMBOL(removeSlot), scope_class->slot_get(SYMBOL(removeSlot)));
      res->slot_set(SYMBOL(updateSlot), scope_class->slot_get(SYMBOL(updateSlot)));
      res->slot_set(SYMBOL(self), self);
      // We really need to copy 'locals' in every scope, or else
      // Scope's methods will get completely fubared: 'locals' will be
      // found in 'self' and will thus not be the local scope!
      res->slot_set(SYMBOL(locals), scope_class->slot_get(SYMBOL(locals)));
      return res;
    }
  }

  rObject
  Object::make_method_scope(const rObject& self)
  {
    rObject res = make_outer_scope(global_class, self);
    res->slot_set(SYMBOL(setSlot), scope_class->slot_get(SYMBOL(setSlot)));
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
    static lookup_result
    slot_lookup(rObject obj, const Object::key_type& k)
    {
      assertion(obj);
      if (obj->own_slot_get(k, 0))
	return obj;
      return boost::optional<rObject>();
    }
  }

  rObject Object::slot_locate(const Object::key_type& k) const
  {
    lookup_action action = boost::bind(slot_lookup, _1, k);
    boost::optional<rObject> res = lookup(action);
    return res ? res.get() : 0;
  }

  rObject
  Object::safe_slot_locate(const key_type& k) const
  {
    rObject r = slot_locate(k);
    if (!r)
      throw LookupError(k);
    return r;
  }

  const rObject&
  Object::slot_get (const key_type& k) const
  {
    rObject cont = safe_slot_locate(k);
    return cont->own_slot_get(k);
  }

  rObject&
  Object::slot_get (const key_type& k)
  {
    return const_cast<rObject&>(const_cast<const Object*>(this)->slot_get(k));
  }


  Object&
  Object::slot_set (const Object::key_type& k, rObject o)
  {
    if (libport::mhas(slots_, k))
      throw RedefinitionError(k);
    slots_[k] = o;
    return *this;
  }

  void
  Object::slot_update (runner::Runner& r,
		       const Object::key_type& k,
		       rObject o)
  {
    // The target of updateSlot
    rObject context = self();
    // The owner of the updated slot
    rObject owner = context->safe_slot_locate(k);

    // We have to determine where the new value must be stored,
    // depending on whether the slot owner and the context are scopes.
    Object* effective_target;

    // If both are scopes, update the original scope.
    if (context->locals_ && owner->locals_get())
      effective_target = owner.get();
    else if (context->locals_ && !owner->locals_get())
    {
      // Local->class: copyonwrite to "self" after evaluating it.
      rObject self = urbi_call(r, context, SYMBOL(self));
      assertion(self);
      effective_target = self.get();
    }
    else // Class->class: copy on write.
      effective_target = context.get();
    // Check hook, only if we are not create-on-writing.
    /* If the current value in the slot to be written in has a slot named
     * 'updateHook', call it, passing the object owning the slot, the slot name
     * and the target. If the return value is not void, write this value in the
     * slot.
     */
    if (effective_target == owner.get())
    {
      rObject& val = owner->own_slot_get(k);
      if (Object* hookOwner = val->slot_locate(SYMBOL(updateHook)).get())
      {
	rObject hook = hookOwner->own_slot_get(SYMBOL(updateHook));
	objects_type args;
	args.push_back(val);
	args.push_back(context);
	args.push_back(String::fresh(k));
	args.push_back(o);
	rObject ret = r.apply(hook, SYMBOL(updateHook), args);
	// If return-value of hook is not void, write it to slot.
	if (ret != object::void_class)
	  effective_target->own_slot_get(k) = ret;
	return;
      }
    }
    effective_target->own_slot_get(k) = o;
  };

  rObject
  Object::own_slot_get (const key_type& k, rObject def)
  {
    return libport::mhas (slots_, k) ? own_slot_get (k) : def;
  }

  void
  Object::all_slots_copy(const rObject& other)
  {
    foreach (object::Object::slot_type slot, other->slots_get())
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
    foreach(Object::slot_type s, slots_)
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
      rObject rthis = self();
      rObject s = urbi_call(runner, rthis, SYMBOL(asString));
      return o << s->value<String>().name_get();
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
} // namespace object
