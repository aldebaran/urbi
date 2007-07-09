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
#define DECLARE(What)				\
  class What;					\
  typedef libport::shared_ptr< What > r ## What;

  DECLARE(Object);
  DECLARE(Float);

} // namespace object

#endif // !OBJECT_FWD_HH
