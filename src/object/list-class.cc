/**
 ** \file object/list-class.cc
 ** \brief Creation of the URBI object list.
 */

#include <libport/foreach.hh>

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
      rList res = new List(l);

      // Append rhs
      PLUS_EQ(res, rhs);
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
    static rList
    tail(rList l)
    {
      List::value_type res(l->value_get());
      res.pop_front();
      return new List(res);
    }

    /// Insert \a elt at the end of \a l
    static rList
    push_back(rList l, rObject elt)
    {
      l->value_get().push_back(elt);
      return l;
    }

    /// Clear \l
    static rList
    clear(rList l)
    {
      l->value_get().clear();
      return l;
    }

    /// Binary predicate used to sort lists.
    static bool
    compareListItems (const rObject a, const rObject b)
    {
      return *a < *b;
    }

    /// \brief Sort a list.
    /// If the list contains different kinds of elements,
    /// the order is not defined.
    /// \return New sorted list
    static rList
    sort (rList l)
    {
      List::value_type res (l->value_get());
      res.sort (compareListItems);
      return new List (res);
    }

    /// Its size.
    static rFloat
    size (rList l)
    {
      return new Float (l->value_get().size());
    }

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
  PRIMITIVE_1_LIST(size);
  PRIMITIVE_1_LIST(sort);
  PRIMITIVE_1_LIST(tail);
  PRIMITIVE_2_LIST(PLUS);
  PRIMITIVE_2_LIST(PLUS_EQ);
  PRIMITIVE_2_OBJECT(list, push_back, List);

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
    DECLARE(push_back);
    DECLARE(size);
    DECLARE(sort);
    DECLARE(tail);
#undef DECLARE
  }

} // namespace object
