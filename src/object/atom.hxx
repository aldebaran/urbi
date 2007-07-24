/**
 ** \file object/atom.hxx
 ** \brief Inline implementation of atoms.
 */

#ifndef OBJECT_ATOM_HXX
# define OBJECT_ATOM_HXX

# include "libport/deref.hh"

# include "ast/pretty-printer.hh"

# include "object/atom.hh"
# include "object/primitives.hh"

/// How to print a UConnection.
/// Used by the atom object::Context.
inline
std::ostream&
operator<< (std::ostream& o, const UConnection& c)
{
  return o << "UConnection_" << static_cast<const void*>(&c);
}

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
	APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(CASE)
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

#endif // !OBJECT_ATOM_HXX
