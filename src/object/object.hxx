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

  inline
  Object::Object ()
  {}

  inline
  Object::~Object ()
  {}

  inline
  rObject&
  Object::operator[] (const Object::key_type& k)
  {
    try
    {
      return lookup (k);
    }
    catch (std::exception e)
    {
      return slots_[k];
    }
  }

  inline
  Object&
  Object::update_slot (const Object::key_type& k, rObject& o)
  {
    lookup (k) = o;
    return *this;
  }

  inline
  Object&
  Object::set_slot (const Object::key_type& k, rObject& o)
  {
    slots_[k] = o;
    return *this;
  }

  inline
  Object&
  Object::remove_slot (const Object::key_type& k)
  {
    slots_.erase (k);
    return *this;
  }

  inline
  std::string
  Object::kind_get () const
  {
    return "Object";
  }

  inline
  Object&
  Object::parent_add (const parent_rtype& p)
  {
    if (parents_.end () == find (parents_.begin (), parents_.end (), p))
      parents_.push_back (p);
    return *this;
  }

  inline
  Object&
  Object::parent_remove (const parent_rtype& p)
  {
    parents_.remove (p);
    return *this;
  }

  inline
  std::ostream&
  operator<< (std::ostream& o, const Object& v)
  {
    return v.dump (o);
  }

} // namespace object

#endif // !OBJECT_OBJECT_HXX
