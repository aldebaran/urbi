/**
 ** \file object/object.cc
 ** \brief Implementation of object::Object.
 */

#include <algorithm>
#include <boost/foreach.hpp>
#include "object/object.hh"
#include "object/atom.hh"

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
#define CASE(Kind) case kind_ ## Kind: return #Kind; break
	CASE(object);
	CASE(float);
	CASE(integer);
	CASE(string);
#undef CASE
      }
    pabort("unreachable");
  }

  /*--------.
  | Slots.  |
  `--------*/

  const rObject&
  Object::lookup (const key_type& k) const
  {
    lookup_set_type lu;
    return lookup(k, lu);
  }

  rObject&
  Object::lookup (const key_type& k)
  {
    return const_cast<rObject&>(const_cast<const Object*>(this)->lookup(k));
  }

  const rObject&
  Object::lookup (const key_type& k, Object::lookup_set_type& lu) const
  {
    /// Look in local slots.
    slots_type::const_iterator it = slots_.find (k);
    if (it != slots_.end ())
      return it->second;

    if (lu.find (this) != lu.end ())
      throw std::exception ();
    lu.insert (this);

    /// Look in parent slots (depth first search)
    BOOST_FOREACH (parent_type p, parents_)
      try
      {
	return p->lookup (k, lu);
      }
      catch (std::exception)
      { }
    /// If not found, throw exception
    throw std::exception ();
  }

  std::ostream&
  Object::special_slots_dump (std::ostream& o) const
  {
    return o;
  }

  std::ostream&
  Object::id_dump (std::ostream& o) const
  {
    try
    {
      // Should be an rString.
      o << lookup("type").cast<String>()->value_get ();
    }
    catch (std::exception e)
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
	o << "parents : ";
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
    for (slots_type::const_iterator i = slots_.begin ();
	 i != slots_.end (); ++i)
      o << i->first << " : " << *i->second << libport::iendl;
    o << libport::decindent << "}";
    return o;
  }

} // namespace object
