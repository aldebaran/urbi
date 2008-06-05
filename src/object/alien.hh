/**
 ** \file object/alien.hh
 ** \brief Alien utilities
 */

#ifndef OBJECT_ALIEN_HH
# define OBJECT_ALIEN_HH

# include <boost/type_traits/remove_reference.hpp>

# include <object/fwd.hh>

namespace object
{

  /// Build an alien
  template<typename T>
  rObject box_with_type(const T&, const std::string&);

  #define box(Type, V)			\
    object::box_with_type<boost::remove_reference<Type>::type>(V, #Type)

  /// Extract an alien content
  template<typename T>
  T unbox_with_type(const rObject&);

  #define unbox(Type, O)			\
    object::unbox_with_type<Type>(O)

} // namespace object

# include <object/alien.hxx>

#endif // OBJECT_ALIEN_HH
