/**
 ** \file object/list-class.cc
 ** \brief Creation of the URBI object list.
 */

#include <boost/foreach.hpp>

#include "object/list-class.hh"
#include "object/atom.hh"
#include "object/object.hh"
#include "primitives.hh"

namespace object
{
  rObject list_class;

  /*----------------------------.
  | Primitives implementation.  |
  `----------------------------*/

  namespace
  {
    /// Concatenate a list in place
    /**
     * @return \a lhs
     */
    static rList
    ip_concat(rList lhs, rList rhs)
    {
      // FIXME: I can't explain why, but the line below result in an
      // infinite loop. Use foreach instead for now.
      //    res.insert(res.end(), rhs->value_get().begin(), rhs->value_get().end());

      BOOST_FOREACH (const rObject& o, rhs->value_get())
	lhs->value_get().push_back(o);

      return lhs;
    }
    /// Concatenate two list
    /**
     * @return A fresh list, concatenation of \a lhs and \a rhs
     */
    static rList
    concat(rList lhs, rList rhs)
    {
      // Copy lhs
      list_traits::type l(lhs->value_get());
      rList res = new List(l);

      // Append rhs
      ip_concat(res, rhs);
      return res;
    }

    /// Give the first element of \a l.
    static rObject
    front(rList l)
    {
      return l->value_get().front();
    }

    /// Give the last element of \a l.
    static rObject
    back(rList l)
    {
      return l->value_get().back();
    }

    /// Give \a l without the first element.
    static rObject
    tail(rList l)
    {
      List::traits::type res(l->value_get());
      res.pop_front();
      return new List(res);
    }

    /// Insert \a elt at the end of \a l
    static rObject
    insert(rList l, rObject elt)
    {
      l->value_get().push_back(elt);
      return l;
    }

    /// Binary predicate used to sort lists.
    static bool
    compareListItems (const rObject a, const rObject b)
    {
      return *a < *b;
    }

    /// Sort a list.
    static rObject
    sort (rList l)
    {
      l->value_get ().sort (compareListItems);
      return l;
    }

  }

  /*-------------------------.
  | Primitives declaration.  |
  `-------------------------*/

#define PRIMITIVE_1_LIST(Name)                  \
  PRIMITIVE_1(list, Name, Name, List)

#define PRIMITIVE_2_LIST(Name, Type2)           \
  PRIMITIVE_2(list, Name, Name, List, Type2)

  PRIMITIVE_2_LIST(concat, List);
  PRIMITIVE_2_LIST(ip_concat, List);
  PRIMITIVE_1_LIST(front);
  PRIMITIVE_1_LIST(back);
  PRIMITIVE_1_LIST(tail);

  PRIMITIVE_2_OBJECT(list, insert, insert, List);

  PRIMITIVE_1_LIST (sort);

  void
  list_class_initialize ()
  {
#define DECLARE(Name, Implem)                   \
    DECLARE_PRIMITIVE(list, Name, Implem)

    DECLARE(+, concat);
    DECLARE(front, front);
    DECLARE(head, front);
    DECLARE(tail, tail);
    DECLARE(back, back);
    //DECLARE(insert, insert);
    Primitive* p = new Primitive (list_class_insert);
    list_class->slot_set ("insert", p);
    DECLARE (sort, sort);
#undef DECLARE
  }

}; // namespace object
