/**
 ** \file object/primitives.hh
 ** \brief Definition of the root Objects.
 */

#ifndef OBJECT_PRIMITIVES_HH
# define OBJECT_PRIMITIVES_HH

# include "object/fwd.hh"

# include "object/code-class.hh"
# include "object/context-class.hh"
# include "object/float-class.hh"
# include "object/integer-class.hh"
# include "object/list-class.hh"
# include "object/object-class.hh"
# include "object/primitive-class.hh"
# include "object/string-class.hh"

namespace object
{
  void root_classes_initialize ();
}; // namespace object

#endif // !OBJECT_PRIMITIVES_HH
