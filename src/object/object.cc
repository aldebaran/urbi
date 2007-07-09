/**
 ** \file object/object.cc
 ** \brief Implementation of object::Object.
 */

#include "object/object.hh"

namespace object
{

  std::ostream& 
  Object::print (std::ostream& o) const
  {
    for (slots_type::const_iterator i = slots_.begin ();
	 i != slots_.end (); ++i)
      o << i->first << " -> " << i->second << libport::iendl;
    return o;
  }


} // namespace object
