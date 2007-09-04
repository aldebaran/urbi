/**
 ** \file object/atom.hxx
 ** \brief Inline implementation of atoms.
 */

#ifndef OBJECT_ATOM_HXX
# define OBJECT_ATOM_HXX

# include <boost/foreach.hpp>

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
  Atom<Traits>::Atom (const typename Traits::type v)
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
  typename libport::ref_traits<typename Traits::type>::type
  Atom<Traits>::value_get ()
  {
    return value_;
  }

  template <typename Traits>
  inline
  std::ostream&
  Atom<Traits>::print(std::ostream& o) const
  {
    assert(0);
    return o;
  }

  template <>
  inline
  std::ostream&
  Atom<float_traits>::print(std::ostream& out) const
  {
    // FIXME: std::fixed leaks to every use of os.
    out << std::fixed << value_get();
    return out;
  }

  template <>
  inline
  std::ostream&
  Atom<list_traits>::print(std::ostream& out) const
  {
    std::list<rObject> values = value_get();
    out << '[';
    bool first = true;
    BOOST_FOREACH (const rObject& o, values)
    {
      if (first)
        first = false;
      else
        out << ", ";
      o->print(out);
    }
    out << ']';
    return out;
  }

  template <>
  inline
  std::ostream&
  Atom<context_traits>::print(std::ostream& out) const
  {
    // FIXME: For now, don't print anything.
    return out;
  }

  template <>
  inline
  std::ostream&
  Atom<code_traits>::print(std::ostream& out) const
  {
    // FIXME: For now, don't print anything.
    return out;
  }

  template <>
  inline
  std::ostream&
  Atom<integer_traits>::print(std::ostream& out) const
  {
    out << value_get();
    return out;
  }

  template <>
  inline
  std::ostream&
  Atom<primitive_traits>::print(std::ostream& out) const
  {
    // FIXME
    assert(!"Printing primitives isn't handled!");
    return out;
  }

  template <>
  inline
  std::ostream&
  Atom<string_traits>::print(std::ostream& out) const
  {
    out << "\"" << value_get() << "\"";
    return out;
  }

  template <typename Traits>
  inline
  std::ostream&
  Atom<Traits>::special_slots_dump (std::ostream& o) const
  {
    return o << "value" << " = " << libport::deref << value_ << libport::iendl;
  }

  template <>
  inline
  std::ostream&
  Atom<list_traits>::special_slots_dump (std::ostream& o) const
  {
    o << "value" << " = " << libport::deref;
    print(o);
    return o << libport::iendl;
  }

} // namespace object

#endif // !OBJECT_ATOM_HXX
