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
# include <libport/intrusive-ptr.hh>

# include <object/object.hh>

namespace object
{

  inline
  Object::~Object ()
  {
    slots_.finalize(this);
    if (!protos_cache_)
      delete protos_;
  }


  /*---------.
  | Protos.  |
  `---------*/

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
  Object::own_slot_get(const key_type& k) const
  {
    return slots_.get(this, k);
  }

  inline
  Object&
  Object::slot_remove(const key_type& k)
  {
    slots_.erase(this, k);
    return *this;
  }

  inline const Object::slots_implem&
  Object::slots_get () const
  {
    return slots_;
  }


  /*--------.
  | Clone.  |
  `--------*/

  inline rObject
  Object::clone() const
  {
    rObject res = new Object;
    // FIXME: clone should not be const!  This was already noted,
    // did the const come back at some point?
    res->proto_add(const_cast<Object*>(this));
    return res;
  }


  /*--------------------------.
  | Free standing functions.  |
  `--------------------------*/

  template<class F> bool
  for_all_protos(const rObject& r, F& f, objects_type& objects)
  {
    if (libport::has(objects, r))
      return false;
    if (f(r))
      return true;
    objects.push_back(r);
    foreach(const rObject& p, r->protos_get())
      if (for_all_protos(p, f, objects))
	return true;
    return false;
  }
  template<class F> bool
  for_all_protos(const rObject& r, F f)
  {
    objects_type objects;
    return for_all_protos(r, f, objects);
  }

  inline const rObject&
  to_boolean(bool b)
  {
    return b ? true_class : false_class;
  }

  template<typename T>
  inline bool
  is_a(const rObject& c)
  {
    // Atom case.
    if (dynamic_cast<T*>(c.get()))
      return true;
    // Through protos case.
    return is_a(c, T::proto);
  }

  /*-------------------.
  | Type conversions.  |
  `-------------------*/

  template <typename T>
  bool Object::is_a() const
  {
    return dynamic_cast<const T*>(this);
  }

  template <typename T>
  libport::intrusive_ptr<T> Object::as() const
  {
    return dynamic_cast<const T*>(this);
  }

  template <typename T>
  libport::intrusive_ptr<T> Object::as()
  {
    return dynamic_cast<T*>(this);
  }

} // namespace object

#endif // !OBJECT_OBJECT_HXX
