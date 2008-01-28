/**
 ** \file object/alien.hh
 ** \brief Alien utilities
 */

#ifndef OBJECT_ALIEN_HH
# define OBJECT_ALIEN_HH

# include "object/alien-class.hh"

namespace object
{

  /// Build an alien
  template<typename T>
  rObject box (const T&);

  /// Extract an alien content
  template<typename T>
  T unbox (const rObject&);

} // namespace object

# include "object/alien.hxx"

#endif // OBJECT_ALIEN_HH
