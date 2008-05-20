/**
 ** \file object/atom.hxx
 ** \brief Inline implementation of atoms.
 */

#ifndef OBJECT_ATOM_HXX
# define OBJECT_ATOM_HXX

# include <boost/foreach.hpp>
# include <boost/lexical_cast.hpp>

# include "libport/deref.hh"
# include "libport/escape.hh"

# include "ast/pretty-printer.hh"

# include "object/atom.hh"
# include "object/primitives.hh"
# include "object/alien-class.hh"
# include "object/code-class.hh"
# include "object/delegate-class.hh"
# include "object/dictionary-class.hh"
# include "object/global-class.hh"
# include "object/lobby-class.hh"
# include "object/float-class.hh"
# include "object/integer-class.hh"
# include "object/list-class.hh"
# include "object/object-class.hh"
# include "object/primitive-class.hh"
# include "object/scope-class.hh"
# include "object/string-class.hh"
# include "object/tag-class.hh"
# include "object/task-class.hh"

/// How to print a UConnection.
/// Used by the atom object::Lobby.
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
  typename Atom<Traits>::shared_type
  Atom<Traits>::fresh (const typename Traits::type v, bool add_proto)
  {
    shared_type res = new Atom<Traits>(v, add_proto);
    res->self_ = res;
    return res;
  }

  template <typename Traits>
  inline
  typename Atom<Traits>::shared_type
  Atom<Traits>::self() const
  {
    rObject res = super_type::self();
    return res.unsafe_cast<Atom<Traits> >();
  }

  // Protected. Use the static fresh method instead.
  template <typename Traits>
  inline
  Atom<Traits>::Atom (const typename Traits::type v, bool add_proto)
    : value_(v)
  {
    if (add_proto)
      switch (kind_get())
	{
# define CASE(What, Name)                       \
	  case object_kind_ ## What:            \
            proto_add (What ## _class);         \
          break;
	  APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(CASE)
# undef CASE
          case object_kind_object:
	    pabort(kind_get());
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
  Atom<object::alien_traits>::operator< (const Atom& rhs) const
  {
    return this < &rhs;
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
  Atom<object::delegate_traits>::operator< (const Atom& rhs) const
  {
    return this < &rhs;
  }

  template <>
  inline
  bool
  Atom<object::dictionary_traits>::operator< (const Atom& rhs) const
  {
    return this < &rhs;
  }

  template <>
  inline
  bool
  Atom<object::lobby_traits>::operator< (const Atom& rhs) const
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


  /*---------------------.
  | special_slots_dump.  |
  `---------------------*/

  template <typename Traits>
  inline
  std::ostream&
  Atom<Traits>::special_slots_dump (std::ostream& o, runner::Runner&) const
  {
    return o << "value" << " = " << libport::deref << value_ << libport::iendl;
  }

  template <>
  inline
  std::ostream&
  Atom<alien_traits>::special_slots_dump (std::ostream& o,
					  runner::Runner&) const
  {
    return o << "type = " << value_.get<1>() << libport::iendl;
  }

  template <>
  inline
  std::ostream&
  Atom<list_traits>::special_slots_dump (std::ostream& o,
					 runner::Runner& runner) const
  {
    o << "value" << " = " << libport::deref;
    print(o, runner);
    return o << libport::iendl;
  }

  template <>
  inline
  std::ostream&
  Atom<delegate_traits>::special_slots_dump (std::ostream& o,
					     runner::Runner&) const
  {
    return o << "delegate" << libport::iendl;
  }

  /*-------.
  | clone. |
  `-------*/

  template<typename Traits>
  inline rObject
  Atom<Traits>::clone () const
  {
    rObject res = Atom<Traits>::fresh(value_get (), false);
    res->proto_add (self());
    return res;
  }

} // namespace object

#endif // !OBJECT_ATOM_HXX
