/**
 ** \file object/state.hh
 ** \brief Definition of object::State.
 */

#ifndef OBJECT_STATE_HH
# define OBJECT_STATE_HH

# include <iosfwd>

# include <kernel/fwd.hh>
# include <object/fwd.hh>

namespace object
{
  /// Everything a context Urbi object includes.
  /// There is one per Context/UConnection.
  struct State
  {
    explicit State (UConnection& c);
    /// The connection.
    UConnection& connection;
  };

  /// For debugging.
  std::ostream& operator<< (std::ostream& o, const State& s);

}; // namespace object

#endif // !OBJECT_STATE_HH
