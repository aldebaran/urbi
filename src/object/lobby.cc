/**
 ** \file object/lobby-class.cc
 ** \brief Creation of the URBI object lobby.
 */

#include <kernel/uconnection.hh>
#include <kernel/userver.hh>

#include <object/lobby.hh>
#include <object/object.hh>
#include <object/string.hh>

#include <runner/raise.hh>
#include <runner/runner.hh>

namespace object
{
  Lobby::Lobby(value_type value)
    : state_(value)
  {
    proto_add(proto ? proto : object_class);
  }

  static kernel::UConnection* dummy = 0;

  Lobby::Lobby(rLobby)
    : state_(*dummy)
  {
    runner::raise_primitive_error("`Lobby' objects cannot be cloned");
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
    if (proto == this)
      runner::raise_primitive_error("must be called on Lobby derivative");

    check_arg_count(args.size(), 1, 2);
    // Second argument is the tag name.
    std::string tag;
    if (args.size() == 2)
    {
      const rString& name = args[1].unsafe_cast<String>();
      if (!name)
	runner::raise_argument_type_error(1, args[1], String::proto);
      tag = name->value_get();
    }
    const rString& rdata = args[0].unsafe_cast<String>();
    if (!rdata)
      runner::raise_argument_type_error(0, args[0], String::proto);
    const std::string data = rdata->value_get() + "\n";
    state_.connection.send (data.c_str(), data.length(), tag.c_str());
  }

  void
  Lobby::write(const std::string& data)
  {
    if (proto == this)
      runner::raise_primitive_error("must be called on Lobby derivative");

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
    return new Lobby(State(kernel::urbiserver->ghost_connection_get()));
  }

  bool Lobby::lobby_added = CxxObject::add<Lobby>();
  const std::string Lobby::type_name = "Lobby";
  rObject Lobby::proto;


}; // namespace object
