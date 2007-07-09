/**
 ** \file object/object.cc
 ** \brief Implementation of object::Object.
 */

#include "object/object.hh"

namespace object
{

  std::ostream&
  Object::special_slots_dump (std::ostream& o) const
  {
    return o;
  }

  std::ostream&
  Object::dump (std::ostream& o) const
  {
    o << kind_get() << " { " << libport::incendl;
    special_slots_dump (o);
    for (slots_type::const_iterator i = slots_.begin ();
	 i != slots_.end (); ++i)
      o << i->first << " -> " << *i->second << libport::iendl;
    o << libport::decindent
      << "}";
    return o;
  }


} // namespace object
