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
    /// Concatenate two list
    /**
     * @return A fresh list, concatenation of \a lhs and \a rhs
     */
    static rList
    concat(rList lhs, rList rhs)
    {
      // Copy lhs
      list_traits::type res(lhs->value_get());

      // Append rhs

      // FIXME: I can't explain why, but the line below result in an
      // infinite loop. Use foreach instead for now.

      //    res.insert(res.end(), rhs->value_get().begin(), rhs->value_get().end());
      BOOST_FOREACH (const rObject& o, rhs->value_get())
        res.push_back(o);

      return new List(res);
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

  }

  /*-------------------------.
  | Primitives declaration.  |
  `-------------------------*/

#define PRIMITIVE_1_LIST(Name)                  \
  PRIMITIVE_1(list, Name, Name, List)

#define PRIMITIVE_2_LIST(Name, Type2)           \
  PRIMITIVE_2(list, Name, Name, List, Type2)

  PRIMITIVE_2_LIST(concat, List);
  PRIMITIVE_1_LIST(front);
  PRIMITIVE_1_LIST(back);
  PRIMITIVE_1_LIST(tail);

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
#undef DECLARE
  }

}; // namespace object
