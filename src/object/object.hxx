/**
 ** \file object/object.hxx
 ** \brief Definition of object::Object.
 */

#ifndef OBJECT_OBJECT_HXX
# define OBJECT_OBJECT_HXX

//#define ENABLE_DEBUG_TRACES
//#include "libport/compiler.hh"

# include <ostream>

# include "libport/containers.hh"
# include "libport/indent.hh"
# include "libport/foreach.hh"

# include "object/object.hh"
# include "object/primitives.hh"


namespace object
{

  inline
  Object::Object ()
    : protos_ (), slots_ (), locals_ (false)
  {
    root_classes_initialize();
  }

  inline
  rObject Object::fresh ()
  {
    rObject res = new Object();
    res->self_ = res;
    return res;
  }

  inline
  rObject Object::self() const
  {
    rObject res;
    res = self_.lock();
    return res;
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
    // FIXME: Is this really faster than using dynamic_cast? Or RTTI?
    return kind_is(object::Object::kind_type(Type::kind));
  }



  /*---------.
  | Protos.  |
  `---------*/

  inline
  Object&
  Object::proto_add (const rObject& p)
  {
    assert(p);
    if (!libport::has(protos_, p))
      protos_.push_front (p);
    return *this;
  }

  inline
  Object&
  Object::proto_remove (const rObject& p)
  {
    assert(p);
    protos_.remove (p);
    return *this;
  }

  inline
  const Object::protos_type&
  Object::protos_get () const
  {
    return protos_;
  }


  /*--------.
  | Slots.  |
  `--------*/

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

  inline rObject
  Object::do_clone (rObject self) const
  {
    rObject res = Object::fresh();
    res->proto_add (self);
    return res;
  }

  /*--------------------------.
  | Free standing functions.  |
  `--------------------------*/

  inline
  rObject
  clone (rObject proto)
  {
    return proto->do_clone (proto);
  }
  template<class P, class F> bool
  for_all_protos(P& r, F& f, objects_type& objects)
  {
    if (libport::has(objects, r))
      return false;
    if (f(r))
      return true;
    objects.push_back(r);
    foreach(P& p, r->protos_get())
      if (for_all_protos(p, f, objects))
	return true;
    return false;
  }
  template<class P,class F> bool
  for_all_protos(P& r, F& f)
  {
    objects_type objects;
    return for_all_protos(r, f, objects);
  }

} // namespace object

#endif // !OBJECT_OBJECT_HXX
