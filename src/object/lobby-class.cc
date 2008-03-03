/**
 ** \file object/lobby-class.cc
 ** \brief Creation of the URBI object lobby.
 */

#include "object/lobby-class.hh"
#include "object/object.hh"

#include "kernel/uconnection.hh"

#include "runner/runner.hh"

namespace object
{
  rObject lobby_class;

  /*------------------.
  | Lobby primitives.  |
  `------------------*/

  static rObject
  lobby_class_asString(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    static rObject lobbyString = new String(libport::Symbol("<lobby>"));
    return lobbyString;
  }

  // Get the current lobby
  static rObject
  lobby_class_self (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return r.lobby_get ();
  }

  static rObject
  lobby_class_send(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT_RANGE (2, 3);
    // Second argument is the tag name.
    std::string tag;
    if (args.size() == 3)
    {
      FETCH_ARG(2, String);
      tag = arg2->value_get().name_get();
    }
    FETCH_ARG(0, Lobby);
    arg0->value_get().connection.send (args[1], tag.c_str(), "");
    return void_class;
  }

  void
  lobby_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(lobby, Name)
    DECLARE (asString);
    DECLARE (self);
    DECLARE (send);
#undef DECLARE
  }

}; // namespace object
