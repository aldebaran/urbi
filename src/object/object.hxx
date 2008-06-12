/**
 ** \file object/object.hxx
 ** \brief Definition of object::Object.
 */

#ifndef OBJECT_OBJECT_HXX
# define OBJECT_OBJECT_HXX

//#define ENABLE_DEBUG_TRACES
//#include <libport/compiler.hh>

# include <ostream>

# include <libport/containers.hh>
# include <libport/indent.hh>
# include <libport/foreach.hh>
# include <libport/shared-ptr.hh>

# include <object/object.hh>
# include <object/list-class.hh>
# include <object/root-classes.hh>
# include <object/primitives.hh>

namespace object
{

  inline
  Object::Object ()
    : protos_ (new protos_type), slots_ (), locals_ (false)
  {
    root_classes_initialize();
  }

  inline
  rObject
  Object::self() const
  {
    return const_cast<Object*>(this);
  }

  inline
  Object::~Object ()
  {
    if (!protos_cache_)
      delete protos_;
  }


  /*-------.
  | kind.  |
  `-------*/

  inline
  Object::kind_type
  Object::kind_get () const
  {
    return object_kind_object;
  }

  inline
  bool
  Object::kind_is(kind_type k) const
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
    return kind_is(kind_type(Type::kind));
  }



  /*---------.
  | Protos.  |
  `---------*/

  inline
  Object&
  Object::proto_add (const rObject& p)
  {
    assert(p);
    /*
      Inheriting from atoms is a problem: when trying to fetch the
      value of an atom, only the local object is inspected, not its
      prototypes. Thus, calling an atom method on an object inheriting
      from this atom systematically fails with a type checking error.

      For now, we forbid inheriting from atoms. This problem could be
      solved by performing a real lookup to find the atom value, but
      we can't tell whether we really want this right now.

      Note that an Object can still inherit from an atom if it is
      itself an atom of the same kind (this pattern is used when
      literals are evaluated, for instance).
    */
    switch(p->kind_get())
    {
#define FORBID(L, U)                                                    \
      case object_kind_ ## L:                                           \
        if (kind_get() != object_kind_ ## L)                            \
          pabort("You can't inherit from atoms for now. Atom type: "    \
                 #U ".");                                               \
      break;
      APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(FORBID)
#undef FORBID
      case object_kind_object:
        // Nothing. Inheriting from objects is Ok.
        break;
    }

    if (!libport::has(*protos_, p))
      protos_->push_front (p);
    return *this;
  }

  inline
  Object&
  Object::proto_remove (const rObject& p)
  {
    assert(p);
    protos_type::iterator i = protos_->begin();
    while (i != protos_->end())
      if (*i == p)
	i = protos_->erase(i);
      else
	++i;
    return *this;
  }

  inline
  const Object::protos_type&
  Object::protos_get () const
  {
    return *protos_;
  }


  /*--------.
  | Slots.  |
  `--------*/

  inline
  rObject
  Object::own_slot_get (const Slots::key_type& k) const
  {
    return slots_.get(k);
  }

  inline
  Object&
  Object::slot_remove (const Slots::key_type& k)
  {
    slots_.erase (k);
    return *this;
  }

  inline
  const Object::slots_implem::content_type&
  Object::slots_get () const
  {
    return slots_.container();
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
  Object::clone() const
  {
    rObject res = new Object();
    res->proto_add(self());
    return res;
  }


  /*--------.
  | Atoms.  |
  `--------*/

  template <typename T>
  inline
  typename T::value_type
  Object::value()
  {
    // This local variable seems to be needed by GCC 4.0.1 on OSX.
    rObject s = self();
    TYPE_CHECK(s, T);
    return s.unsafe_cast<T>()->value_get();
  }


  /*--------------------------.
  | Free standing functions.  |
  `--------------------------*/

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

  inline rObject
  to_boolean(bool b)
  {
    return b ? true_class : false_class;
  }

} // namespace object

#endif // !OBJECT_OBJECT_HXX
