/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/lobby.hh
 ** \brief Definition of the Urbi object lobby.
 */

#ifndef OBJECT_LOBBY_HH
# define OBJECT_LOBBY_HH

# include <libport/compiler.hh>
# include <libport/instance-tracker.hh>

# include <kernel/fwd.hh>

# include <urbi/object/cxx-object.hh>
# include <urbi/object/fwd.hh>

namespace urbi
{
  namespace object
  {
    class Lobby: public CxxObject, public libport::InstanceTracker<Lobby>
    {
    public:
      typedef kernel::UConnection connection_type;

      /// Convenience constructor.
      /// UConnection::send requires a non-const connection.
      Lobby(connection_type* v);

      /// Must not be called, lobbies cannot be cloned.
      ATTRIBUTE_NORETURN Lobby(rLobby model);

      /// The connectionTag.
      rTag tag_get() const;

      void send(const objects_type& args);
      void write(const std::string& data);
      connection_type& connection_get();
      const connection_type& connection_get() const;

      /// Create new lobby attached to a ghost connection.
      rLobby create();

      /// All the existing lobbies.
      typedef libport::InstanceTracker<Lobby>::set_type lobbies_type;

      /// The lobby we depend on.
      static rLobby lobby();

      /// Mark this lobby as disconnected.
      void disconnect();

      /// Shut the lobby/connection down.
      void quit();

      /// Fake reception of a string on the connection.
      void receive(const std::string& data);

      size_t bytesSent() const;
      size_t bytesReceived() const;
    private:
      /// The Lobby prototype uses an empty connection_.
      /// The actual lobbies must have a non-empty one.
      connection_type* connection_;

      URBI_CXX_OBJECT_(Lobby);
    };
  }; // namespace object
}

# include <urbi/object/lobby.hxx>

#endif // !OBJECT_LOBBY_HH
