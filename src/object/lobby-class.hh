/**
 ** \file object/lobby-class.hh
 ** \brief Definition of the URBI object lobby.
 */

#ifndef OBJECT_LOBBY_CLASS_HH
# define OBJECT_LOBBY_CLASS_HH

# include <object/fwd.hh>

namespace object
{
  /// The prototype for Lobby objects.
  extern rObject lobby_class;

  /// Initialize the Lobby class.
  void lobby_class_initialize ();
}; // namespace object

#endif // !OBJECT_LOBBY_CLASS_HH
