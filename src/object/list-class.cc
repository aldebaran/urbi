/**
 ** \file object/list-class.cc
 ** \brief Creation of the URBI object list.
 */

#include <boost/bind.hpp>

#include <libport/foreach.hh>
#include <libport/ufloat.hh>

#include "kernel/userver.hh"
#include "object/float-class.hh"
#include "object/list-class.hh"
#include "object/atom.hh"
#include "object/object.hh"
#include "primitives.hh"
#include "runner/runner.hh"

namespace object
{
  rObject list_class;

  /*----------------------------.
  | Primitives implementation.  |
  `----------------------------*/

  namespace
  {
    /// Concatenate two lists in place.
    /**
     * @return \a lhs
     */
    static rList
    PLUS_EQ(rList lhs, rList rhs)
    {
      // FIXME: If we use the following code, then a += a does not
      // end.
      //
      //      lhs->value_get().insert(lhs->value_get().end(),
      //			      rhs->value_get().begin(),
      //			      rhs->value_get().end());

      foreach (const rObject& o, rhs->value_get())
	lhs->value_get().push_back(o);

      return lhs;
    }

    /// Concatenate two lists.
    /**
     * @return A fresh list, concatenation of \a lhs and \a rhs
     */
    static rList
    PLUS(rList lhs, rList rhs)
    {
      // Copy lhs
      list_traits::type l(lhs->value_get());
      rList res = List::fresh(l);

      // Append rhs
      PLUS_EQ(res, rhs);
      return res;
    }

#define CHECK_NON_EMPTY							\
    List::value_type& ul = l->value_get();				\
    if (ul.empty ())							\
      throw PrimitiveError						\
	(__FUNCTION__, "operation cannot be applied onto empty list")

    /// Give the first element of \a l.
    static rObject
    front(rList l)
    {
      CHECK_NON_EMPTY;
      return ul.front();
    }

    /// Give the last element of \a l.
    static rObject
    back(rList l)
    {
      CHECK_NON_EMPTY;
      return ul.back();
    }

    /// Give \a l without the first element.
    static rList
    tail(rList l)
    {
      CHECK_NON_EMPTY;
      List::value_type res = ul;
      res.pop_front();
      return List::fresh(res);
    }

    /// Insert \a elt at the end of \a l
    static rList
    push_back(rList l, rObject elt)
    {
      l->value_get().push_back(elt);
      return l;
    }

    /// Insert \a elt at the front of \a l
    static rList
    push_front(rList l, rObject elt)
    {
      l->value_get().push_front(elt);
      return l;
    }

    /// Remove first element from \a l
    static rList
    pop_front(rList l)
    {
      CHECK_NON_EMPTY;
      ul.pop_front();
      return l;
    }

    static rList
    removeById(rList l, rObject elt)
    {
      List::value_type under = l->value_get();
      List::value_type::iterator i = under.begin();
      while (i != under.end())
	if (*i == elt)
	  i = under.erase(i);
	else
	  ++i;
      l->value_set(under);
      return l;
    }

#undef CHECK_NON_EMPTY

    /// Clear \l
    static rList
    clear(rList l)
    {
      l->value_get().clear();
      return l;
    }

    /// Binary predicate used to sort lists.
    static bool
    compareListItems (runner::Runner&, rObject a, rObject b)
    {
      objects_type args;
      args.push_back(b);
      return is_true(urbi_call(::urbiserver->getCurrentRunner(),
                               a, SYMBOL(LT), args));
    }

    /// \brief Sort a list.
    /// If the list contains different kinds of elements,
    /// the order is not defined.
    /// \return New sorted list
    static rObject
    list_class_sort (runner::Runner& r, objects_type args)
    {
      CHECK_ARG_COUNT(1);
      FETCH_ARG(0, List);
      std::list<rObject> s;
      foreach(const rObject& o, arg0->value_get())
	s.push_back(o);
      s.sort(boost::bind(compareListItems, boost::ref(r), _1, _2));

      List::value_type res;
      foreach(const rObject& o, s)
	res.push_back(o);
      return List::fresh(res);
    }

    /// Its size.
    static rFloat
    size (rList l)
    {
      return Float::fresh(l->value_get().size());
    }

  }

  static rObject
  list_class_nth(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    FETCH_ARG(0, List);
    FETCH_ARG(1, Float);
    int index = ufloat_to_int(arg1->value_get(), "nth");
    if (index < 0 || index >= static_cast<int>(arg0->value_get().size()))
      throw PrimitiveError("nth", "invalid index");
    return arg0->value_get().at(index);
  }

  /*-------------------------.
  | Primitives declaration.  |
  `-------------------------*/

#define PRIMITIVE_1_LIST(Name)                  \
  PRIMITIVE_1(list, Name, List)

#define PRIMITIVE_2_LIST(Name)			\
  PRIMITIVE_2(list, Name, List, List)

  PRIMITIVE_1_LIST(back);
  PRIMITIVE_1_LIST(clear);
  PRIMITIVE_1_LIST(front);
  PRIMITIVE_1_LIST(pop_front);
  PRIMITIVE_1_LIST(size);
  PRIMITIVE_1_LIST(tail);
  PRIMITIVE_2_LIST(PLUS);
  PRIMITIVE_2_LIST(PLUS_EQ);
  PRIMITIVE_2_OBJECT(list, push_back, List);
  PRIMITIVE_2_OBJECT(list, push_front, List);
  PRIMITIVE_2_OBJECT(list, removeById, List);

  void
  list_class_initialize ()
  {
#define DECLARE(Name)                   \
    DECLARE_PRIMITIVE(list, Name)

    DECLARE(PLUS);
    DECLARE(PLUS_EQ);
    DECLARE(back);
    DECLARE(clear);
    DECLARE(front);
    DECLARE(nth);
    DECLARE(pop_front);
    DECLARE(push_back);
    DECLARE(push_front);
    DECLARE(removeById);
    DECLARE(size);
    DECLARE(sort);
    DECLARE(tail);
#undef DECLARE
  }

} // namespace object
