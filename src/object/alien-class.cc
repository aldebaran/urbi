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
    alien_class->slot_set (#Name,					\
			  new Primitive(alien_class_ ## Name));
#undef DECLARE
  }

}; // namespace object
