/*
 * Copyright (C) 2007-2012, Gostai S.A.S.
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

#include <urbi/kernel/uconnection.hh>
#include <kernel/ughostconnection.hh>
#include <urbi/kernel/userver.hh>

#include <urbi/object/lobby.hh>
#include <urbi/object/object.hh>
#include <urbi/object/string.hh>
#include <urbi/object/symbols.hh>
#include <urbi/object/tag.hh>

#include <urbi/runner/raise.hh>
#include <runner/runner.hh>
#include <runner/shell.hh>

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
        tag_get()->name_set(libport::Symbol::fresh_string("Lobby"));
      }
    }

    Lobby::Lobby(rLobby)
      : connection_(0)
    {
      RAISE("`Lobby' objects cannot be cloned");
    }

    URBI_CXX_OBJECT_INIT(Lobby)
      : connection_(0)
    {
      bind(SYMBOL(send),
           static_cast<void (Lobby::*)(const std::string&)>(&Lobby::send));
      bind(SYMBOL(send),
           static_cast<void (Lobby::*)(const std::string&, const std::string&)>
             (&Lobby::send));

#define DECLARE(Name)                  \
      bind(SYMBOL_(Name), &Lobby::Name)

      DECLARE(binaryMode);
      DECLARE(bytesSent);
      DECLARE(bytesReceived);
      DECLARE(create);
      DECLARE(lobby);
      DECLARE(quit);
      DECLARE(receive);
      DECLARE(write);

#undef DECLARE

      bind(SYMBOL(instances), &Lobby::instances_get);
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

#define REQUIRE_DERIVATIVE_AND_CONNECTION()             \
    do {                                                \
      if (proto == this)                                \
        RAISE("must be called on Lobby derivative");    \
      if (!connection_)                                 \
        return;                                         \
    } while (false)

    void
    Lobby::quit()
    {
      REQUIRE_DERIVATIVE_AND_CONNECTION();
      connection_->close();
    }

    void
    Lobby::send(const std::string& data)
    {
      send(data, "");
    }

    void
    Lobby::send(const std::string& data, const std::string& tag)
    {
      REQUIRE_DERIVATIVE_AND_CONNECTION();
      connection_->send(data + "\n", tag.c_str());
    }

    void
    Lobby::write(const std::string& data)
    {
      REQUIRE_DERIVATIVE_AND_CONNECTION();
      connection_->send_queue(data.c_str(), data.size());
      connection_->flush();
    }

    void
    Lobby::receive(const std::string& s)
    {
      connection_->received(s);
    }

    void
    Lobby::binaryMode(bool m, const std::string& tag)
    {
      runner::Shell& s = dynamic_cast<runner::Shell&>(::kernel::runner());
      s.setSerializationMode(m, tag);
    }
  } // namespace object
}
