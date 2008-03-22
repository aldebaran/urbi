/**
 ** \file object/integer-class.cc
 ** \brief Creation of the URBI object integer.
 */

#include <boost/lexical_cast.hpp>
#include "object/integer-class.hh"
#include "object/object.hh"
#include "object/atom.hh"

namespace object
{
  rObject integer_class;

  /*---------------------.
  | Integer primitives.  |
  `---------------------*/

  static rObject
  integer_class_asString(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    if (args[0] == integer_class)
      return String::fresh(SYMBOL(LT_Integer_GT));
    return String::fresh(libport::Symbol(boost::lexical_cast<std::string>
		      (VALUE(args[0], Integer))));
  }

  void
  integer_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(integer, Name)
    DECLARE (asString);
#undef DECLARE
  }

}; // namespace object
