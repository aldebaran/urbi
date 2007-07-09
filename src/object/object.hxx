/**
 ** \file object/object.hxx
 ** \brief Definition of object::Object.
 */

#ifndef OBJECT_OBJECT_HXX
# define OBJECT_OBJECT_HXX

# include <ostream>
# include "libport/indent.hh"
# include "object/object.hh"

namespace object
{

  Object::Object ()
  {}

  Object::~Object ()
  {}
  
  rObject& 
  Object::operator[] (const Object::key_type& k)
  {
    return slots_[k];
  }

  std::ostream& 
  Object::print (std::ostream& o) const
  {
    for (slots_type::const_iterator i = slots_.begin ();
	 i != slots_.end (); ++i)
      o << i->first << " -> " << i->second << libport::iendl;
    return o;
  }

  std::ostream&
  operator<< (std::ostream& o, const Object& v)
  {
    return v.print (o);
  }

} // namespace object

#endif // !OBJECT_OBJECT_HXX
