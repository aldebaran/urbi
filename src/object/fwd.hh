/**
 ** \file object/fwd.hh
 ** \brief Forward declarations of all node-classes of OBJECT
 ** (needed by the visitors)
 */
#ifndef OBJECT_FWD_HH
# define OBJECT_FWD_HH

# include "libport/fwd.hh"
# include "libport/shared-ptr.hh"

namespace object
{
# define DECLARE(What)				\
  class What;					\
  typedef libport::shared_ptr< What > r ## What

  DECLARE(Object);
# undef DECLARE

  template <typename Traits>
  class Atom;

# define DECLARE(What, Name)				\
  struct What ## _traits;				\
  typedef Atom< What ## _traits > Name;			\
  typedef libport::shared_ptr < Name > r ## Name

  DECLARE(float, Float);
  DECLARE(integer, Integer);
  DECLARE(string, String);

# undef DECLARE


} // namespace object

#endif // !OBJECT_FWD_HH
