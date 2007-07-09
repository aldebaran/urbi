/**
 ** \file object/atom.cc
 ** \brief Definition of object::Float, object::Integer, object::String.
 */

#include "object/atom.hh"

namespace object
{

  /*--------.
  | Float.  |
  `--------*/
  const char float_traits::prefix[] = "Float";

  /*----------.
  | Integer.  |
  `----------*/
  const char integer_traits::prefix[] = "Integer";

  /*---------.
  | String.  |
  `---------*/
  const char string_traits::prefix[] = "String";

} // namespace object
