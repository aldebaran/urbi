/**
 ** \file object/object.cc
 ** \brief Implementation of object::Object.
 */

#include <algorithm>

#include <boost/lexical_cast.hpp>

#include <libport/containers.hh>
#include <libport/foreach.hh>

#include "object/object.hh"
#include "object/atom.hh"
#include "object/urbi-exception.hh"

#include "runner/runner.hh"

namespace object
{
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

  /*--------.
  | Slots.  |
  `--------*/

  Object::locate_type
  Object::slot_locate (const key_type& k, Object::objects_type& os) const
  {
    /// Look in local slots.
    if (libport::mhas(slots_, k))
      return locate_type(true, rObject());

    /// Break recursive loops.
    if (libport::mhas(os, this))
      return locate_type(false, rObject());
    os.insert (this);

    /// Look in proto slots (depth first search).
    locate_type res;
    foreach(rObject p, protos_)
      if ((res = p->slot_locate(k, os)).first)
	return res.second?res:locate_type(true, p);
    return locate_type(false, rObject());
  }

  rObject slot_locate(const rObject& ref, const Object::key_type& k)
  {
    Object::objects_type os;
    Object::locate_type l = ref->slot_locate(k, os);
    if (l.first)
      return l.second? l.second:ref;
    else
      return rObject();
  }

  Object*
  Object::slot_locate(const key_type& k) const
  {
    objects_type os;
    Object::locate_type l = slot_locate(k, os);
    if (l.first)
      return const_cast<Object*>(l.second?l.second.get():this);
    else
      return 0;
  }

  Object&
  Object::safe_slot_locate(const key_type& k) const
  {
    Object* r = slot_locate(k);
    if (!r)
      boost::throw_exception (LookupError(k));
    return *r;
  }

  const rObject&
  Object::slot_get (const key_type& k) const
  {
    Object& cont = safe_slot_locate(k);
    return cont.own_slot_get(k);
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
      boost::throw_exception (RedefinitionError(k));
    slots_[k] = o;
    return *this;
  }

  void
  slot_update (runner::Runner& r,
	       rObject& context,
	       const Object::key_type& k,
	       rObject o)
  {
    Object& l = context->safe_slot_locate(k);

    if (context->locals_ && l.locals_)  // Local scope writes local var: no copyonwrite.
      l.own_slot_get(k) = o;
    else if (context->locals_ && !l.locals_)
    {
      // Local->class: copyonwrite to "self" after evaluating it.
      rObject self_obj = context->slot_get(SYMBOL(self));
      assert (self_obj);
      objects_type self_args;
      self_args.push_back (context);
      rObject self = r.apply (self_obj, self_args);
      assert(self);
      self.get ()->slots_[k] = o;
    }
    else // Class->class: copy on write.
      context->slots_[k] = o;
  };

  rObject
  Object::own_slot_get (const key_type& k, rObject def)
  {
    if (libport::mhas (slots_, k))
      return own_slot_get (k);
    return def;
  }

  /*-----------.
  | Printing.  |
  `-----------*/

  std::ostream&
  Object::special_slots_dump (std::ostream& o) const
  {
    return o;
  }

  bool
  Object::operator< (const Object& rhs) const
  {
    return this < &rhs;
  }

  std::ostream&
  Object::id_dump (std::ostream& o) const
  {
    try
    {
      // Should be an rString.
      o << slot_get(SYMBOL(type)).cast<String>()->value_get ();
    }
    catch (UrbiException&)
    {}
    return o << '_' << this;
  }


  std::ostream&
  Object::dump (std::ostream& o) const
  {
    id_dump (o);
    /// Use xalloc/iword to store our current depth within the stream object.
    static const long idx = o.xalloc();
    static const long depth_max = 3;
    long& current_depth = o.iword(idx);
    /// Stop recursion at depth_max.
    if (current_depth > depth_max)
      return o << " <...>";
    ++current_depth;
    o << " {" << libport::incendl;
    if (protos_.begin () != protos_.end ())
      {
	o << "protos = ";
	for (protos_type::const_iterator i = protos_.begin ();
	     i != protos_.end (); ++i)
	  {
	    if (i != protos_.begin())
	      o << ", ";
	    (*i)->id_dump (o);
	  }
	o << libport::iendl;
      }
    special_slots_dump (o);
    foreach (slot_type s, slots_)
      o << s << libport::iendl;
    o << libport::decindent << '}';
    //We can not reuse current_depth variable above according to spec.
    o.iword(idx)--;
    return o;
  }

  std::ostream&
  Object::print(std::ostream& out) const
  {
    // Temporary hack, detect void and print nothing
    if (this == void_class.get())
      return out;
    // FIXME: Decide what should be printed, but at least print something
    return out << "<object_" << std::hex << (long) (this) << ">";
  }

} // namespace object
