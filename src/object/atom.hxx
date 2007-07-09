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
  std::ostream&
  Atom<Traits>::print (std::ostream& o) const
  {
    o << Traits::prefix << " { " << libport::incendl
      << "value = " << value_;
    Object::print (o);
    o << libport::decendl
      << "}";
    return o;
  }

} // namespace object

# include "object/atom.hxx"

#endif // !OBJECT_ATOM_HH
