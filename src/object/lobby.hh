/**
 ** \file object/lobby-class.hh
 ** \brief Definition of the URBI object lobby.
 */

#ifndef OBJECT_LOBBY_CLASS_HH
# define OBJECT_LOBBY_CLASS_HH

# include <libport/compiler.hh>
# include <libport/instance-tracker.hh>

# include <object/cxx-object.hh>
# include <object/fwd.hh>
# include <object/state.hh>

namespace object
{
  class Lobby: public CxxObject, public libport::InstanceTracker<Lobby>
  {
  public:
    typedef State value_type;

    ATTRIBUTE_NORETURN Lobby(rLobby model);
    Lobby(value_type value);

    void send(const objects_type& args);
    void write(const std::string& data);
    value_type& value_get();
    const value_type& value_get() const;

  private:
    value_type state_;

    URBI_CXX_OBJECT(Lobby);
  };
}; // namespace object

#endif // !OBJECT_LOBBY_CLASS_HH
