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

  private:
    /// The Lobby prototype uses an empty connection_.
    /// The actual lobbies must have a non-empty one.
    connection_type* connection_;

    URBI_CXX_OBJECT(Lobby);
  };
}; // namespace object

#endif // !OBJECT_LOBBY_CLASS_HH
