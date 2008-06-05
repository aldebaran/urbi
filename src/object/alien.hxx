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
    return Alien::fresh(boost::make_tuple(v, type));
  }

  template<typename T>
  inline T
  unbox_with_type(const rObject& o)
  {
    TYPE_CHECK(o, Alien);
    rAlien a = o.unsafe_cast<Alien>();
    // GCC 4.0.1 on OSX cannot grok the following version:
    //    a->value_get().get<0>();
    return boost::any_cast<T>(boost::get<0>(a->value_get()));
  }

} // namespace object

#endif // OBJECT_ALIEN_HXX
