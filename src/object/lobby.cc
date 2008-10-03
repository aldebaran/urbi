/**
 ** \file object/lobby-class.cc
 ** \brief Creation of the URBI object lobby.
 */

#include <kernel/uconnection.hh>
#include <kernel/userver.hh>

#include <object/lobby.hh>
#include <object/object.hh>
#include <object/string.hh>

#include <runner/runner.hh>

namespace object
{
  Lobby::Lobby(value_type value)
    : state_(value)
  {
    proto_add(proto ? proto : object_class);
  }

  static UConnection* dummy = 0;

  Lobby::Lobby(rLobby)
    : state_(*dummy)
  {
    throw PrimitiveError(SYMBOL(clone), "cloning a lobby is invalid");
  }

  Lobby::value_type&
  Lobby::value_get()
  {
    return state_;
  }

  const Lobby::value_type&
  Lobby::value_get() const
  {
    return state_;
  }

  void
  Lobby::send(objects_type& args)
  {
    check_arg_count(args.size(), 1, 2);
    // Second argument is the tag name.
    std::string tag;
    if (args.size() == 2)
    {
      const rString& name = args[1].unsafe_cast<String>();
      if (!name)
	throw WrongArgumentType("String", "Object", SYMBOL(send));
      tag = name->value_get();
    }
    const rString& rdata = args[0].unsafe_cast<String>();
    if (!rdata)
      throw WrongArgumentType("String", "Object", SYMBOL(send));
    const std::string data = rdata->value_get() + "\n";
    state_.connection.send (data.c_str(), data.length(), tag.c_str());
  }

  void
  Lobby::write(const std::string& data)
  {
    state_.connection.send_queue(data.c_str(),
				 data.size());
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

  rObject
  Lobby::proto_make()
  {
    return new Lobby(State(::urbiserver->ghost_connection_get()));
  }

  bool Lobby::lobby_added = CxxObject::add<Lobby>();
  const std::string Lobby::type_name = "Lobby";
  rObject Lobby::proto;


}; // namespace object
