/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/lobby.cc
 ** \brief Creation of the Urbi object lobby.
 */

#include <libport/cassert>

#include <kernel/uconnection.hh>
#include <kernel/ughostconnection.hh>
#include <kernel/userver.hh>

#include <urbi/object/lobby.hh>
#include <urbi/object/object.hh>
#include <urbi/object/string.hh>
#include <object/symbols.hh>
#include <urbi/object/tag.hh>

#include <urbi/runner/raise.hh>
#include <runner/runner.hh>

namespace urbi
{
  namespace object
  {
    Lobby::Lobby(connection_type* c)
      : connection_(c)
    {
      // Only the Lobby prototype is expected to have a null connection.
      aver(!proto || c);
      proto_add(proto ? rObject(proto) : Object::proto);

      if (c)
      {
        // Initialize the connection tag used to reference local
        // variables.
        slot_set(SYMBOL(connectionTag), new Tag());
        tag_get()->name_set(libport::Symbol::fresh("Lobby"));
      }
    }

    Lobby::Lobby(rLobby)
      : connection_(0)
    {
      RAISE("`Lobby' objects cannot be cloned");
    }

    size_t
    Lobby::bytesSent() const
    {
      if (!connection_)
        RAISE("Lobby is not connected");
      return connection_->bytes_sent();
    }

    size_t
    Lobby::bytesReceived() const
    {
      if (!connection_)
        RAISE("Lobby is not connected");
      return connection_->bytes_received();
    }

    rLobby
    Lobby::create()
    {
      return
        (new kernel::UGhostConnection(*kernel::urbiserver, true))
        ->lobby_get();
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
    Lobby::disconnect()
    {
      connection_ = 0;
      call(SYMBOL(handleDisconnect));
    }

    rLobby
    Lobby::lobby()
    {
      // Don't return "this", as any "Lobby.lobby" must return the
      // lobby of the connection, not the target of the ".lobby" call.
      return ::kernel::runner().lobby_get();
    }

    void
    Lobby::quit()
    {
      if (proto == this)
        RAISE("must be called on Lobby derivative");
      if (&connection_get())
        connection_get().close();
    }

    void
    Lobby::send(const objects_type& args)
    {
      if (proto == this)
        RAISE("must be called on Lobby derivative");

      check_arg_count(args.size(), 1, 2);
      if (!connection_)
        return;
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
        RAISE("must be called on Lobby derivative");
      if (!connection_)
        return;
      connection_->send_queue(data.c_str(), data.size());
      connection_->flush();
    }

    void
    Lobby::receive(const std::string& s)
    {
      connection_->received(s);
    }

    void
    Lobby::initialize(CxxObject::Binder<Lobby>& bind)
    {
#define DECLARE(Name)                           \
      bind(SYMBOL(Name), &Lobby::Name)
      DECLARE(bytesSent);
      DECLARE(bytesReceived);
      DECLARE(create);
      DECLARE(lobby);
      DECLARE(quit);
      DECLARE(receive);
      DECLARE(send);
      DECLARE(write);
#undef DECLARE

      bind(SYMBOL(instances), &Lobby::instances_get);
    }


    URBI_CXX_OBJECT_REGISTER(Lobby)
      : connection_(0)
    {}

  }; // namespace object
}
