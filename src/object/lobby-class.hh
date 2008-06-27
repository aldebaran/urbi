/**
 ** \file object/lobby-class.hh
 ** \brief Definition of the URBI object lobby.
 */

#ifndef OBJECT_LOBBY_CLASS_HH
# define OBJECT_LOBBY_CLASS_HH

# include <object/cxx-object.hh>
# include <object/fwd.hh>

namespace object
{
  extern rObject lobby_class;

  class Lobby: public CxxObject
  {
  public:
    typedef State value_type;

    Lobby();
    Lobby(rLobby model);
    Lobby(value_type value);

    void send(objects_type args);
    value_type& value_get();

    static const std::string type_name;
    virtual std::string type_name_get() const;

  private:
    value_type state_;

  public:
    static void initialize(CxxObject::Binder<Lobby>& binder);
    static bool lobby_added;

  };
}; // namespace object

#endif // !OBJECT_LOBBY_CLASS_HH
