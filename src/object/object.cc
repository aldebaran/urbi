/**
 ** \file object/object.cc
 ** \brief Implementation of object::Object.
 */

#include "object/object.hh"

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
    for (parents_type::const_iterator i = parents_.begin ();
	 i != parents_.end (); ++i)
      try
      {
	const rObject& tmp_lookup = (*i)->lookup (k, lu);
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
    return o;
  }

  std::ostream&
  Object::id_dump (std::ostream& o) const
  {
    try
    {
      // Should be an rString that knows how to print itself.
      lookup("type")->special_slots_dump (o);
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

  namespace
  {
    /// Create the Object class.
    static
    rObject
    new_object_class ()
    {
      // It has no parents, and for the time being, no contents.
      return new Object;
    }

    /// Create the Float class.
    static
    rObject
    new_float_class (rObject object_class)
    {
      rObject res = clone(object_class);
      // res["type"] = new String ("Float");
      return res;
    }

    /// Create the Integer class.
    static
    rObject
    new_integer_class (rObject object_class)
    {
      rObject res = clone(object_class);
      // res["type"] = new String ("Integer");
      return res;
    }

    /// Create the Float class.
    static
    rObject
    new_string_class (rObject object_class)
    {
      rObject res = clone(object_class);
      // res["type"] = new String ("String");
      return res;
    }
  }

  rObject object_class = new_object_class();
  rObject string_class = new_string_class(object_class);
  rObject float_class = new_float_class(object_class);
  rObject integer_class = new_integer_class(object_class);

} // namespace object
