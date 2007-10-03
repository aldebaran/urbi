/**
 ** \file object/object.cc
 ** \brief Implementation of object::Object.
 */

#include <algorithm>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "libport/containers.hh"

#include "object/object.hh"
#include "object/atom.hh"
#include "object/urbi-exception.hh"

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

  const Object*
  Object::which (const key_type& k, Object::objects_type& os) const
  {
    /// Look in local slots.
    slots_type::const_iterator it = slots_.find (k);
    if (libport::mhas(slots_, k))
      return this;

    /// Break recursive loops.
    if (libport::mhas(os, this))
      return 0;
    os.insert (this);

    /// Look in parent slots (depth first search).
    BOOST_FOREACH (rObject p, parents_)
      if (const Object* res = p->which (k, os))
	return res;
    return 0;
  }

  const Object*
  Object::which (const key_type& k) const
  {
    objects_type os;
    return which(k, os);
  }

  const rObject&
  Object::lookup (const key_type& k) const
  {
    if (const Object* o = which(k))
      return o->own_slot_get (k);
    else
      throw LookupError(k);
  }

  rObject&
  Object::lookup (const key_type& k)
  {
    return const_cast<rObject&>(const_cast<const Object*>(this)->lookup(k));
  }


  Object&
  Object::slot_set (const Object::key_type& k, rObject o)
  {
    if (libport::mhas(slots_, k))
      throw RedefinitionError(k);
    slots_[k] = o;
    return *this;
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
      o << lookup("type").cast<String>()->value_get ();
    }
    catch (UrbiException&)
    {}
    return o << '_' << this;
  }

  std::ostream&
  Object::dump (std::ostream& o) const
  {
    id_dump (o);
    o << " {" << libport::incendl;
    if (parents_.begin () != parents_.end ())
      {
	o << "parents = ";
	for (parents_type::const_iterator i = parents_.begin ();
	     i != parents_.end (); ++i)
	  {
	    if (i != parents_.begin())
	      o << ", ";
	    (*i)->id_dump (o);
	  }
	o << libport::iendl;
      }
    special_slots_dump (o);
    BOOST_FOREACH (slot_type s, slots_)
      o << s << libport::iendl;
    o << libport::decindent << '}';
    return o;
  }

  std::ostream&
  Object::print(std::ostream& out) const
  {
    // FIXME: For now, don't print anything.
    return out;
  }

} // namespace object
