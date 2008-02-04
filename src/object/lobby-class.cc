/**
 ** \file object/lobby-class.cc
 ** \brief Creation of the URBI object lobby.
 */

#include "object/lobby-class.hh"
#include "object/object.hh"

#include "runner/runner.hh"

namespace object
{
  rObject lobby_class;

  /*------------------.
  | Lobby primitives.  |
  `------------------*/

  // Get the current lobby
  static rObject
  lobby_class_self (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return r.lobby_get ();
  }

  void
  lobby_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(lobby, Name, Name)
    DECLARE (self);
#undef DECLARE
  }

}; // namespace object
