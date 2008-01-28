/**
 ** \file object/alien.hxx
 ** \brief Alien utilities
 */

#ifndef OBJECT_ALIEN_HXX
# define OBJECT_ALIEN_HXX

# include "object/atom.hh"
# include "object/primitives.hh"

namespace object
{

  /*----------------.
  | Alien helpers.  |
  `----------------*/

  template<typename T>
  inline rObject
  box (const T& value)
  {
    return new Alien (value);
  }

  template<typename T>
  inline T
  unbox (const rObject& o)
  {
    TYPE_CHECK (o, Alien);
    rAlien a = o.unsafe_cast<Alien> ();
    return boost::any_cast<T> (a->value_get ());
  }

} // namespace object

#endif // OBJECT_ALIEN_HXX
