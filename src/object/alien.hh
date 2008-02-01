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
  rObject box_with_type(const T&, const std::string&);

  #define box(type, v)			\
    object::box_with_type(v, #type)

  /// Extract an alien content
  template<typename T>
  T unbox_with_type(const rObject&);

  #define unbox(type, o)			\
    object::unbox_with_type<type>(o)

} // namespace object

# include "object/alien.hxx"

#endif // OBJECT_ALIEN_HH
