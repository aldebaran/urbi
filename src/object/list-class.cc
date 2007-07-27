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
    static const list_traits::type
    list_concat(const list_traits::type& lhs,
                const list_traits::type& rhs)
    {
      // Copy lhs
      list_traits::type res(lhs);

      // Append rhs

      // FIXME: I can't explain why, but the line below result in an
      // infinite loop. Use foreach instead for now.

      //    res.insert(res.end(), rhs->value_get().begin(), rhs->value_get().end());
      BOOST_FOREACH (const rObject& o, rhs)
        res.push_back(o);

      return res;
    }
  }

  /*-------------------------.
  | Primitives declaration.  |
  `-------------------------*/

  PRIMITIVE_2(list, concat, list_concat, List, List, List);

  void
  list_class_initialize ()
  {
    DECLARE_PRIMITIVE(list, +, concat);
  }

}; // namespace object
