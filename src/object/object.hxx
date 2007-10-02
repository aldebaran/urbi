/**
 ** \file object/object.hxx
 ** \brief Definition of object::Object.
 */

#ifndef OBJECT_OBJECT_HXX
# define OBJECT_OBJECT_HXX

//#define ENABLE_DEBUG_TRACES
//#include "libport/compiler.hh"

# include <ostream>

# include "libport/indent.hh"

# include "object/object.hh"
# include "object/primitives.hh"

namespace object
{

  inline
  Object::Object ()
    : parents_ (), slots_ (), locals_ (false)
  {
    root_classes_initialize();
  }

  inline
  Object::~Object ()
  {}


  /*-------.
  | kind.  |
  `-------*/

  inline
  Object::kind_type
  Object::kind_get () const
  {
    return kind_object;
  }

  inline
  bool
  Object::kind_is(Object::kind_type k) const
  {
    return kind_get () == k;
  }

  template <typename Type>
  inline
  bool
  Object::type_is() const
  {
    // FIXME: static_assert Obj derives from Object.
    // FIXME: Is this really faster than using dynamic_cast? Or RTTI.
    return kind_is(object::Object::kind_type(Type::traits::kind));
  }



  /*----------.
  | Parents.  |
  `----------*/

  inline
  Object&
  Object::parent_add (const parent_type& p)
  {
    assert(p);
    if (parents_.end () == find (parents_.begin (), parents_.end (), p))
      parents_.push_back (p);
    return *this;
  }

  inline
  Object&
  Object::parent_remove (const parent_type& p)
  {
    assert(p);
    parents_.remove (p);
    return *this;
  }

  inline
  const Object::parents_type&
  Object::parents_get () const
  {
    return parents_;
  }


  /*--------.
  | Slots.  |
  `--------*/

  inline
  Object&
  Object::slot_update (const Object::key_type& k, rObject o)
  {
    if (locals_)
      lookup(k) = o;
    else
      slots_[k] = o;
    return *this;
  }

  inline
  rObject&
  Object::own_slot_get (const Object::key_type& k)
  {
    return slots_[k];
  }

  inline
  const rObject&
  Object::own_slot_get (const Object::key_type& k) const
  {
    return const_cast<Object*>(this)->slots_[k];
  }

  inline
  Object&
  Object::slot_remove (const Object::key_type& k)
  {
    slots_.erase (k);
    return *this;
  }

  inline
  const Object::slots_type&
  Object::slots_get () const
  {
    return slots_;
  }


  /*-------------.
  | Properties.  |
  `-------------*/

  inline
  bool
  Object::locals_get () const
  {
    return locals_;
  }

  inline
  Object&
  Object::locals_set (bool b)
  {
    locals_ = b;
    return *this;
  }


  /*--------------------------.
  | Free standing functions.  |
  `--------------------------*/

  inline
  rObject
  clone (rObject ref)
  {
    rObject res = new Object;
    res->parent_add (ref);
    return res;
  }

  inline
  std::ostream&
  operator<< (std::ostream& o, const Object& v)
  {
    return v.dump (o);
  }

  inline
  std::ostream&
  operator<< (std::ostream& o, const Object::slot_type& s)
  {
    return o << s.first << " = " << *s.second;
  }

} // namespace object

#endif // !OBJECT_OBJECT_HXX
