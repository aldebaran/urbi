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
  const char float_traits::kind[] = "Float";

  /*----------.
  | Integer.  |
  `----------*/
  const char integer_traits::kind[] = "Integer";

  /*---------.
  | String.  |
  `---------*/
  const char string_traits::kind[] = "String";

} // namespace object
