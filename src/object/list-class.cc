/**
 ** \file object/list-class.cc
 ** \brief Creation of the URBI object list.
 */

#include <boost/foreach.hpp>

#include "object/list-class.hh"
#include "object/atom.hh"
#include "object/object.hh"

namespace object
{
  rObject list_class;

  /*------------------.
  | List primitives.  |
  `------------------*/

  /// Concatenate two list
  /**
   * @return A fresh list, concatenation of \a lsh and \a rhs
   */
  static rList list_concat(rList& lhs, const rList& rhs)
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

  rObject
  list_class_ip_concat (objects_type args)
  {
    assert(args[0]->kind_get() == Object::kind_list);
    assert(args[1]->kind_get() == Object::kind_list);
    rList lhs = args[0].unsafe_cast<List>();
    rList rhs = args[1].unsafe_cast<List>();
    rList res = list_concat(lhs, rhs);
    return res;
  }


  /// Initialize the List class.
  void
  list_class_initialize ()
  {
    list_class->slot_set ("+",
                          new Primitive(list_class_ip_concat));
  }

}; // namespace object
