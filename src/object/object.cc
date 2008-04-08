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
		    rObject& self,
		    libport::Symbol msg,
		    objects_type& args)
  {
    assertion(self);
    rObject message = self->slot_get(msg);
    if (!message)
      throw LookupError(msg);
    args.insert(args.begin(), self);
    rObject res = r.apply(message, args);
    args.erase(args.begin());
    return res;
  }

  rObject urbi_call(runner::Runner& r,
		    rObject& self,
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
  typedef std::pair<boost::optional<rObject>, bool> lookup_result;
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
      if (obj->own_slot_get(slotName, 0))
	// Return a nonempty optional containing an empty rObject, to
	// indicate to target that the lookup is successful, and the
	// target is the initial object.
	return make_pair(optional<rObject>(rObject()), false);
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
	      return make_pair(code->own_slot_get(SYMBOL(context)), false);
	    else
	      foreach (const rObject& var, VALUE(captured, object::List))
		if (VALUE(var, object::String) == slotName)
		  return make_pair(target(code->own_slot_get(SYMBOL(context)), slotName), false);
	  }
	if (self->slot_locate(slotName))
	  return make_pair(self, false);
      }
      return make_pair(optional<rObject>(), true);
    }
  }

  rObject
  target(rObject where, const libport::Symbol& name)
  {
    boost::function1<std::pair<boost::optional<rObject>, bool>, rObject> lookup =
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
      try
      {
	res->slot_set(SYMBOL(getSlot), scope_class->slot_get(SYMBOL(getSlot)));
	res->slot_set(SYMBOL(locateSlot), scope_class->slot_get(SYMBOL(locateSlot)));
	res->slot_set(SYMBOL(removeSlot), scope_class->slot_get(SYMBOL(removeSlot)));
	res->slot_set(SYMBOL(updateSlot), scope_class->slot_get(SYMBOL(updateSlot)));
	res->slot_set(SYMBOL(self), self);
	// We really need to copy 'locals' in every scope, or else
	// Scope's methods will get completely fubared: 'locals' will be
	// found in 'self' and will thus not be the local scope!
	res->slot_set(SYMBOL(locals), scope_class->slot_get(SYMBOL(locals)));
      }
      catch (object::LookupError&)
      {
	// Nothing. This never happens in normal use, since all these
	// scope methods are defined at the very top of
	// urbi/urbi.u. Yet, a first scope is created to load urbi.u,
	// and these method are thus not yet defined. We define the
	// target slot by hand so as anything written in urbi.u goes in
	// Lobby. So keep in mind that urbi.u is evaluated a little bit
	// differently than a regular lobby: every {set,get,update}Slot
	// goes in Lobby, not the local scope - which is quite
	// reasonable, since this scope will never be usable again.
      }
      return res;
    }
  }

  rObject
  Object::make_method_scope(const rObject& self)
  {
    rObject res = make_outer_scope(global_class, self);
    try
    {
      res->slot_set(SYMBOL(setSlot), scope_class->slot_get(SYMBOL(setSlot)));
    }
    catch (object::LookupError&)
    {
      // Nothing. See comment in make_outer_scope's catch
    }
    return res;
  }

  rObject
  Object::make_do_scope(const rObject& parent, const rObject& self)
  {
    rObject res = make_outer_scope (parent ? parent : global_class, self);
    try
    {
      res->slot_set(SYMBOL(setSlot), scope_class->slot_get(SYMBOL(doSetSlot)));
    }
    catch (LookupError&)
    {
      // Nothing. See comment in make_outer_scope's catch
    }
    return res;
  }

  /*--------.
  | Slots.  |
  `--------*/

  template <typename R>
  boost::optional<R>
  Object::lookup(boost::function1<std::pair<boost::optional<R>, bool>,
				  rObject> action,
		 objects_set_type& marks) const
  {
    if (!libport::mhas(marks, this))
    {
      marks.insert(this);
      assertion(self());
      std::pair<boost::optional<R>, bool> res = action(self());
      if (res.first)
	return res.first;
      else
	if (res.second)
        {
	  foreach (const rObject& proto, protos_get())
	    if (boost::optional<R> res = proto->lookup(action, marks))
	      return res;
        }
    }
    return boost::optional<R>();
  }

  template <typename R>
  boost::optional<R>
  Object::lookup(boost::function1<std::pair<boost::optional<R>, bool>,
				  rObject> action) const
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
	return std::make_pair(obj, false);
      return std::make_pair(boost::optional<rObject>(), true);
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
  slot_update (runner::Runner& r,
	       rObject& context,
	       const Object::key_type& k,
	       rObject o)
  {
    // The owner of the updated slot
    rObject owner = context->safe_slot_locate(k);

    // We have to determine where the new value must be stored,
    // depending on whether the slot owner and the context are scopes.

    // If both are scopes, update the original scope.
    if (context->locals_ && owner->locals_get())
      owner->own_slot_get(k) = o;
    else if (context->locals_ && !owner->locals_get())
    {
      // Local->class: copyonwrite to "self" after evaluating it.
      rObject self_obj = context->slot_get(SYMBOL(self));
      assertion(self_obj);
      objects_type self_args;
      self_args.push_back (context);
      rObject self = r.apply (self_obj, self_args);
      assertion(self);
      self->slot_set(k, o);
    }
    // If the context isn't a scope, copy on write.
    else
      context->slots_[k] = o;
  };

  rObject
  Object::own_slot_get (const key_type& k, rObject def)
  {
    return libport::mhas (slots_, k) ? own_slot_get (k) : def;
  }

  void Object::all_slots_copy(const rObject& other)
  {
    foreach (object::Object::slot_type slot, other->slots_get())
      if (!own_slot_get(slot.first, 0))
	slot_set(slot.first, slot.second);
  }

  /*-----------.
  | Printing.  |
  `-----------*/

  std::ostream&
  Object::special_slots_dump (runner::Runner&, rObject, std::ostream& o) const
  {
    return o;
  }

  bool
  Object::operator< (const Object& rhs) const
  {
    return this < &rhs;
  }

  // FIXME: A smart pointer to this (\a self) is required for now to
  // avoid deleting this at the end of the method.
  std::ostream&
  Object::id_dump (const rObject& self,
		   std::ostream& o,
		   runner::Runner& r)
  {
    rObject id = self->slot_get(SYMBOL(id));
    objects_type id_args;
    id_args.push_back(self);
    rObject data = r.apply(id, id_args);
    std::string s = VALUE(data, String).name_get();
    return o << s;
  }

  std::ostream&
  dump (runner::Runner& runner, rObject r, std::ostream& o)
  {
    r->id_dump (r, o, runner);
    /// Use xalloc/iword to store our current depth within the stream object.
    static const long idx = o.xalloc();
    static const long depth_max = 3;
    long& current_depth = o.iword(idx);
    /// Stop recursion at depth_max.
    if (current_depth > depth_max)
      return o << " <...>";
    ++current_depth;
    o << " {" << libport::incendl;
    if (r->protos_.begin () != r->protos_.end ())
      {
	o << "protos = ";
	for (Object::protos_type::const_iterator i = r->protos_.begin ();
	     i != r->protos_.end (); ++i)
	  {
	    if (i != r->protos_.begin())
	      o << ", ";
	    (*i)->id_dump (*i, o, runner);
	  }
	o << libport::iendl;
      }
    r->special_slots_dump (runner, r, o);
    foreach(Object::slot_type s, r->slots_)
    {
      o << s.first << " = ";
      dump(runner, s.second, o) << libport::iendl;
    }
    o << libport::decindent << '}';
    //We can not reuse current_depth variable above according to spec.
    o.iword(idx)--;
    return o;
  }

  std::ostream&
  print(runner::Runner& runner, rObject r, std::ostream& out)
  {
    try
    {
      rObject s = urbi_call(runner, r, SYMBOL(asString));
      out << VALUE(s, String).name_get();

      return out;
    }
    // Check if asString was found, especially for bootstrap: asString
    // is implemented in urbi/urbi.u, but print is called to show
    // result in the toplevel before its definition.
    catch (LookupError&)
    {
      // If no asString method is supplied, print the unique id
      out << std::hex << r.get();
      return out;
    }
  }

  bool
  is_a(const rObject& c, const rObject& p)
  {
    return for_all_protos(c, boost::lambda::_1 == p);
  }
} // namespace object
