/**
 ** \file object/alien.hxx
 ** \brief Alien utilities
 */

#ifndef OBJECT_ALIEN_HXX
# define OBJECT_ALIEN_HXX

# include <boost/any.hpp>
# include <boost/tuple/tuple.hpp>

# include <object/atom.hh>
# include <object/primitives.hh>

namespace object
{

  /*----------------.
  | Alien helpers.  |
  `----------------*/

  template<typename T>
  inline rObject
  box_with_type(const T& v, const std::string& type)
  {
    return new Alien(std::make_pair(v, type));
  }

  template<typename T>
  inline T
  unbox_with_type(const rObject& o)
  {
    TYPE_CHECK(o, Alien);
    rAlien a = o.unsafe_cast<Alien>();
    return boost::any_cast<T>(a->value_get().first);
  }

} // namespace object

#endif // OBJECT_ALIEN_HXX
