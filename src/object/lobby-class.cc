/**
 ** \file object/lobby-class.cc
 ** \brief Creation of the URBI object lobby.
 */

#include "object/lobby-class.hh"
#include "object/object.hh"

#include "kernel/uconnection.hh"

#include "object/scope-class.hh"
#include "runner/runner.hh"

namespace object
{
  rObject lobby_class;

  /*------------------.
  | Lobby primitives.  |
  `------------------*/

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

  static rObject
  lobby_class_target(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    FETCH_ARG(1, String);

    return target(args[0], arg1->value_get());
  }

  void
  lobby_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(lobby, Name)
    DECLARE (send);
    DECLARE (target);
#undef DECLARE
  }

}; // namespace object
