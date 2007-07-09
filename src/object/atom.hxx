/**
 ** \file object/atom.hh
 ** \brief Definition of object::Float.
 */

#ifndef OBJECT_ATOM_HH
# define OBJECT_ATOM_HH

# include "object/atom.hh"

namespace object
{

  template <typename Traits>
  inline
  Atom<Traits>::Atom (const typename Traits::type& v)
    : value_(v)
  {}

  template <typename Traits>
  inline
  Atom<Traits>::~Atom ()
  {}

  template <typename Traits>
  inline
  std::string
  Atom<Traits>::kind_get () const
  {
    return Traits::kind;
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
