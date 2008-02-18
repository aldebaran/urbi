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

  void
  alien_class_initialize ()
  {
#define DECLARE(Name)							\
    DECLARE_PRIMITIVE(alien, Name);
#undef DECLARE
  }

}; // namespace object
