/**
 ** \file object/atom.hxx
 ** \brief Inline implementation of atoms.
 */

#ifndef OBJECT_ATOM_HXX
# define OBJECT_ATOM_HXX

# include <boost/foreach.hpp>

# include "libport/deref.hh"
# include "libport/escape.hh"

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

  /*------------------.
  | value accessors.  |
  `------------------*/

  template <typename Traits>
  inline
  const typename Atom<Traits>::value_type
  Atom<Traits>::value_get () const
  {
    return value_;
  }

  template <typename Traits>
  inline
  typename Atom<Traits>::value_ref_type
  Atom<Traits>::value_get ()
  {
    return value_;
  }

  template <typename Traits>
  inline
  void
  Atom<Traits>::value_set (typename Atom<Traits>::value_ref_type v)
  {
    value_ = v;
  }


  /*------------.
  | operator<.  |
  `------------*/

  template <typename Traits>
  inline
  bool
  Atom<Traits>::operator< (const Atom& rhs) const
  {
    return value_ < rhs.value_get ();
  }

  template <>
  inline
  bool
  Atom<object::code_traits>::operator< (const Atom& rhs) const
  {
    return this < &rhs;
  }

  template <>
  inline
  bool
  Atom<object::context_traits>::operator< (const Atom& rhs) const
  {
    return this < &rhs;
  }

  template <typename Traits>
  inline
  bool
  Atom<Traits>::operator< (const Object& rhs) const
  {
    if (const Atom* a = dynamic_cast<const Atom*> (&rhs))
      return this->operator< (*a);
    else
      return this->Object::operator< (rhs);
  }

  /*--------.
  | print.  |
  `--------*/

  template <typename Traits>
  inline
  std::ostream&
  Atom<Traits>::print(std::ostream& o) const
  {
    pabort("Printing an Atom<T>");
    return o;
  }

  template <>
  inline
  std::ostream&
  Atom<code_traits>::print(std::ostream& out) const
  {
    return out << value_get();
  }

  template <>
  inline
  std::ostream&
  Atom<context_traits>::print(std::ostream& out) const
  {
    // FIXME: discuss what we should print here.
    out << "<context>";
    return out;
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
  Atom<integer_traits>::print(std::ostream& out) const
  {
    out << value_get();
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
  Atom<primitive_traits>::print(std::ostream& out) const
  {
    // FIXME: discuss what we should print here.
    out << "<primitive>";
    return out;
  }

  template <>
  inline
  std::ostream&
  Atom<string_traits>::print(std::ostream& out) const
  {
    out << "\"" << libport::escape(value_get()) << "\"";
    return out;
  }

  /*---------------------.
  | special_slots_dump.  |
  `---------------------*/

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


  /*--------------------------.
  | Free standing functions.  |
  `--------------------------*/

  template <typename Traits>
  typename Atom<Traits>::shared_type 
  clone (typename Atom<Traits>::shared_type ref)
  {
    typename Atom<Traits>::shared_type res = new Atom<Traits>;
    res->parent_add (ref);
    return res;
  }

} // namespace object

#endif // !OBJECT_ATOM_HXX
