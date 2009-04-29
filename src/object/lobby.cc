/**
 ** \file object/lobby-class.cc
 ** \brief Creation of the URBI object lobby.
 */

#include <libport/assert.hh>

#include <kernel/uconnection.hh>
#include <kernel/userver.hh>

#include <object/lobby.hh>
#include <object/object.hh>
#include <object/string.hh>
#include <object/symbols.hh>
#include <object/tag.hh>

#include <runner/raise.hh>
#include <runner/runner.hh>

namespace object
{
  Lobby::Lobby(connection_type* c)
    : connection_(c)
  {
    // Only the Lobby prototype is expected to have a null connection.
    assert(!proto || c);
    proto_add(proto ? proto : Object::proto);

    if (c)
    {
      // Easy reference to the current lobby.
      // ndmefyl: Easy reference my ass, easy memory-leaking cycling reference rather!
      slot_set(SYMBOL(lobby), this);

      // Initialize the connection tag used to reference local
      // variables.
      slot_set
        (SYMBOL(connectionTag),
         new Tag(new sched::Tag(libport::Symbol(c->connection_tag_))));
    }
  }

  Lobby::Lobby(rLobby)
    : connection_(0)
  {
    runner::raise_primitive_error("`Lobby' objects cannot be cloned");
  }

  Lobby::connection_type&
  Lobby::connection_get()
  {
    return *connection_;
  }

  const Lobby::connection_type&
  Lobby::connection_get() const
  {
    return *connection_;
  }

  void
  Lobby::send(const objects_type& args)
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
    connection_->send(data.c_str(), data.length(), tag.c_str());
  }

  void
  Lobby::write(const std::string& data)
  {
    if (proto == this)
      runner::raise_primitive_error("must be called on Lobby derivative");
    connection_->send_queue(data.c_str(), data.size());
    connection_->flush();
  }

  void
  Lobby::initialize(CxxObject::Binder<Lobby>& bind)
  {
    bind(SYMBOL(send), &Lobby::send);
    bind(SYMBOL(write), &Lobby::write);
  }

  rObject
  Lobby::proto_make()
  {
    return new Lobby(0);
  }

  URBI_CXX_OBJECT_REGISTER(Lobby);

}; // namespace object
