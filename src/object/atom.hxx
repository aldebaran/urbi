/**
 ** \file object/atom.hh
 ** \brief Definition of object::Float.
 */

#ifndef OBJECT_ATOM_HH
# define OBJECT_ATOM_HH

# include "libport/deref.hh"

# include "ast/pretty-printer.hh"

# include "object/atom.hh"
# include "object/primitives.hh"

namespace object
{

  template <typename Traits>
  inline
  Atom<Traits>::Atom (const typename Traits::type& v)
    : value_(v)
  {
    switch (kind_get())
      {
# define CASE(What, Name)					\
	case kind_ ## What: parent_add (What ## _class); break;
	APPLY_ON_GLOBAL_PRIMITIVES_BUT_OBJECT(CASE)
# undef CASE
      case kind_object:
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
    // As is usual for integer values, Traits::kind is an enum whose
    // value is a kind_type.  So we have to cast it back to its type
    // here.
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
    return o << "value" << " = " << libport::deref << value_ << libport::iendl;
  }

} // namespace object

# include "object/atom.hxx"

#endif // !OBJECT_ATOM_HH
