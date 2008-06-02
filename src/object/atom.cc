/**
 ** \file object/atom.hxx
 ** \brief Inline implementation of atoms.
 */

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "libport/deref.hh"
#include "libport/escape.hh"

#include "ast/print.hh"

#include "object/atom.hh"
#include "object/primitives.hh"
#include "object/alien-class.hh"
#include "object/code-class.hh"
#include "object/delegate-class.hh"
#include "object/dictionary-class.hh"
#include "object/global-class.hh"
#include "object/lobby-class.hh"
#include "object/float-class.hh"
#include "object/integer-class.hh"
#include "object/list-class.hh"
#include "object/object-class.hh"
#include "object/primitive-class.hh"
#include "object/string-class.hh"
#include "object/tag-class.hh"
#include "object/task-class.hh"
#include "object/urbi-exception.hh"

namespace object
{
  template <typename Traits>
  typename Atom<Traits>::shared_type
  Atom<Traits>::fresh (const typename Traits::type v, bool add_proto)
  {
    shared_type res = new Atom<Traits>(v, add_proto);
    return res;
  }

  template <typename Traits>
  typename Atom<Traits>::shared_type
  Atom<Traits>::self() const
  {
    rObject res = super_type::self();
    return res.unsafe_cast<Atom<Traits> >();
  }

  // Protected. Use the static fresh method instead.
  template <typename Traits>
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
  Atom<Traits>::~Atom ()
  {}

  template <typename Traits>
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
  const typename Atom<Traits>::value_type
  Atom<Traits>::value_get () const
  {
    return value_;
  }

  template <typename Traits>
  typename Atom<Traits>::value_ref_type
  Atom<Traits>::value_get ()
  {
    return value_;
  }

  template <typename Traits>
  void
  Atom<Traits>::value_set (typename Atom<Traits>::value_ref_type v)
  {
    value_ = v;
  }

  template <>
  void
  Atom<code_traits>::value_set (Atom<code_traits>::value_ref_type)
  {
    throw InternalError("cannot call object::Code::value_set");
  }

  template <>
  void
  Atom<lobby_traits>::value_set (Atom<lobby_traits>::value_ref_type)
  {
    throw InternalError("cannot call object::Code::value_set");
  }

  /*------------.
  | operator<.  |
  `------------*/

  template <typename Traits>
  bool
  Atom<Traits>::operator< (const Atom& rhs) const
  {
    return value_ < rhs.value_get ();
  }

  template <>
  bool
  Atom<object::alien_traits>::operator< (const Atom& rhs) const
  {
    return this < &rhs;
  }

  template <>
  bool
  Atom<object::code_traits>::operator< (const Atom& rhs) const
  {
    return this < &rhs;
  }

  template <>
  bool
  Atom<object::delegate_traits>::operator< (const Atom& rhs) const
  {
    return this < &rhs;
  }

  template <>
  bool
  Atom<object::dictionary_traits>::operator< (const Atom& rhs) const
  {
    return this < &rhs;
  }

  template <>
  bool
  Atom<object::lobby_traits>::operator< (const Atom& rhs) const
  {
    return this < &rhs;
  }

  template <typename Traits>
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
  std::ostream&
  Atom<Traits>::special_slots_dump (std::ostream& o, runner::Runner&) const
  {
    return o << "value" << " = " << libport::deref << value_ << libport::iendl;
  }

  template <>
  std::ostream&
  Atom<alien_traits>::special_slots_dump (std::ostream& o,
					  runner::Runner&) const
  {
    return o << "type = " << value_.get<1>() << libport::iendl;
  }

  template <>
  std::ostream&
  Atom<list_traits>::special_slots_dump (std::ostream& o,
					 runner::Runner& runner) const
  {
    o << "value" << " = " << libport::deref;
    print(o, runner);
    return o << libport::iendl;
  }

  template <>
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
  rObject
  Atom<Traits>::clone () const
  {
    rObject res = Atom<Traits>::fresh(value_get (), false);
    res->proto_add (self());
    return res;
  }

  // Force instantiation
#define INSTANTIATE(What, Name) template class Atom<What ## _traits>;
  APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(INSTANTIATE)

} // namespace object
