/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/**
 ** \file object/lobby-class.hh
 ** \brief Definition of the URBI object lobby.
 */

#ifndef OBJECT_LOBBY_CLASS_HH
# define OBJECT_LOBBY_CLASS_HH

# include <libport/compiler.hh>
# include <libport/instance-tracker.hh>

# include <kernel/fwd.hh>

# include <object/cxx-object.hh>
# include <object/fwd.hh>

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

    void send(const objects_type& args);
    void write(const std::string& data);
    connection_type& connection_get();
    const connection_type& connection_get() const;
    /// Mark this lobby as disconnected.
    void disconnect();
    /// Create new lobby attached to a ghost connection.
    rLobby create();
    /// Fake reception of a string on the connection.
    void receive(const std::string& data);
    /// Resend the banner using an urbiScript call to send
    void resendBanner();
  private:
    /// The Lobby prototype uses an empty connection_.
    /// The actual lobbies must have a non-empty one.
    connection_type* connection_;

    URBI_CXX_OBJECT(Lobby);
  };
}; // namespace object

#endif // !OBJECT_LOBBY_CLASS_HH
