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
  box_with_type(const T& v, const std::string& type)
  {
    return new Alien(boost::make_tuple(v, type));
  }

  template<typename T>
  inline T
  unbox_with_type(const rObject& o)
  {
    TYPE_CHECK(o, Alien);
    rAlien a = o.unsafe_cast<Alien>();
    return boost::any_cast<T>(a->value_get().get<0>());
  }

} // namespace object

#endif // OBJECT_ALIEN_HXX
