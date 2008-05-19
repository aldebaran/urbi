#include <libport/assert.hh>

#include "object/object-kind.hh"

namespace object
{
  const char*
  string_of (object_kind_type k)
  {
    switch (k)
    {
#define CASE(What, Name) case object_kind_ ## What: return #Name; break;
      APPLY_ON_ALL_PRIMITIVES(CASE);
#undef CASE
    }
    pabort("unreachable");
  }

}
