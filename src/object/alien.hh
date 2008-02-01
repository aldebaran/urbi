/**
 ** \file object/alien.hh
 ** \brief Alien utilities
 */

#ifndef OBJECT_ALIEN_HH
# define OBJECT_ALIEN_HH

# include <libport/select-ref.hh>

# include "object/fwd.hh"

namespace object
{

  /// Build an alien
  template<typename T>
  rObject box_with_type(const T&, const std::string&);

  #define box(Type, V)			\
    object::box_with_type<libport::unref_traits<Type>::type>(V, #Type)

  /// Extract an alien content
  template<typename T>
  T unbox_with_type(const rObject&);

  #define unbox(Type, O)			\
    object::unbox_with_type<Type>(O)

} // namespace object

# include "object/alien.hxx"

#endif // OBJECT_ALIEN_HH
