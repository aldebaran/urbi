/**
 ** \file object/lobby-class.cc
 ** \brief Creation of the URBI object lobby.
 */

#include <object/lobby-class.hh>
#include <object/object.hh>

#include <kernel/uconnection.hh>

#include <object/atom.hh>
#include <object/string-class.hh>
#include <runner/runner.hh>

namespace object
{
  rObject lobby_class;

  Lobby::Lobby(value_type value)
    : state_(value)
  {
    proto_add(lobby_class);
  }

  static UConnection* dummy = 0;

  Lobby::Lobby()
    : state_(*dummy)
  {
    pabort("Cannot instanciate lobbies by hand");
  }

  Lobby::Lobby(rLobby)
    : state_(*dummy)
  {
    throw WrongArgumentType("foobar");
    pabort("Cannot clone lobbies");
  }

  Lobby::value_type&
  Lobby::value_get()
  {
    return state_;
  }

  void
  Lobby::send(objects_type args)
  {
    CHECK_ARG_COUNT_RANGE (1, 2);
    // Second argument is the tag name.
    std::string tag;
    if (args.size() == 2)
    {
      rString name = args[1].unsafe_cast<String>();
      if (!name)
	throw WrongArgumentType("String", "Object", "send");
      tag = name->value_get().name_get();
    }
    rString rdata = args[0].unsafe_cast<String>();
    if (!rdata)
      throw WrongArgumentType("String", "Object", "send");
    std::string data = rdata->value_get().name_get() + "\n";
    state_.connection.send (data.c_str(), data.length(), tag.c_str());
  }

  void
  Lobby::write(rString data)
  {
    state_.connection.send_queue(data->value_get().name_get().c_str(),
				 data->value_get().name_get().length());
    state_.connection.flush();
  }

  void
  Lobby::initialize(CxxObject::Binder<Lobby>& bind)
  {
    bind(SYMBOL(send), &Lobby::send);
    bind(SYMBOL(write), &Lobby::write);
  }

  std::string
  Lobby::type_name_get() const
  {
    return type_name;
  }

  bool Lobby::lobby_added =
    CxxObject::add<Lobby>("Lobby", lobby_class);
  const std::string Lobby::type_name = "Lobby";

}; // namespace object
