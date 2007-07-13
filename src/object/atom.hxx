/**
 ** \file object/atom.hh
 ** \brief Definition of object::Float.
 */

#ifndef OBJECT_ATOM_HH
# define OBJECT_ATOM_HH

# include "object/atom.hh"
# include "object/primitives.hh"

namespace object
{

  template <typename Traits>
  inline
  Atom<Traits>::Atom (const typename Traits::type& v)
    : value_(v)
  {
    // FIXME: I don't know how do make it better for the time being.
    // Help me if you can.
    switch (kind_get())
      {
# define CASE(Kind) case kind_ ## Kind: parent_add (Kind ## _class); break
	CASE(float);
	CASE(integer);
	CASE(string);
# undef CASE
      default:
	pabort (kind_get());
      }
  }

  template <typename Traits>
  inline
  Atom<Traits>::~Atom ()
  {}

  template <typename Traits>
  inline
  Object::kind_type
  Atom<Traits>::kind_get () const
  {
    return static_cast<Object::kind_type>(Traits::kind);
  }

  template <typename Traits>
  inline
  typename Traits::type
  Atom<Traits>::value_get () const
  {
    return value_;
  }

  template <typename Traits>
  inline
  std::ostream&
  Atom<Traits>::special_slots_dump (std::ostream& o) const
  {
    return o << "value" << " -> " << value_ << libport::iendl;
  }

} // namespace object

# include "object/atom.hxx"

#endif // !OBJECT_ATOM_HH
