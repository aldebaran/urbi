/**
 ** \file object/alien-class.cc
 ** \brief Creation of the URBI object alien.
 */

#include "object/alien-class.hh"

#include "object/atom.hh"
#include "object/object.hh"

#include "runner/runner.hh"

namespace object
{
  rObject alien_class;

  /*-------------------.
  | Alien primitives.  |
  `-------------------*/

  static rObject
  alien_class_asString(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    static rObject alienString = new String(SYMBOL(LT_alien_GT));
    return alienString;
  }

  void
  alien_class_initialize ()
  {
#define DECLARE(Name)							\
    DECLARE_PRIMITIVE(alien, Name);
    DECLARE(asString);
#undef DECLARE
  }

}; // namespace object
