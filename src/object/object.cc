/**
 ** \file object/object.cc
 ** \brief Implementation of object::Object.
 */

#include "object/object.hh"

namespace object
{

  rObject&
  Object::lookup (const key_type& k)
  {
    /// Look in local slots.
    slots_type::iterator it = slots_.find (k);
    if (it != slots_.end ())
      return it->second;
    /// Look in parent slots (depth first search)
    for (parents_type::const_iterator i = parents_.begin ();
	 i != parents_.end (); ++i)
      try
      {
	rObject& tmp_lookup = (*i)->lookup (k);
	return tmp_lookup;
      }
      catch (std::exception)
      { }
    /// If not found, throw exception
    throw std::exception ();
  }

  std::ostream&
  Object::special_slots_dump (std::ostream& o) const
  {
    o << "parents : ";
    if (parents_.begin () != parents_.end ())
    {
      o << libport::incendl;
      for (parents_type::const_iterator i = parents_.begin ();
	   i != parents_.end (); ++i)
	((**i)["name"])->special_slots_dump (o);
      o << libport::decendl;
    }
    else
      o << " <> " << libport::iendl;
    return o;
  }

  std::ostream&
  Object::dump (std::ostream& o) const
  {
    o << kind_get() << '_' << this << " {" << libport::incendl;
    special_slots_dump (o);
    for (slots_type::const_iterator i = slots_.begin ();
	 i != slots_.end (); ++i)
      o << i->first << " -> " << *i->second << libport::iendl;
    o << libport::decindent
      << "}";
    return o;
  }

} // namespace object
