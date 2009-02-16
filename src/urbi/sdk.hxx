#ifndef URBI_SDK_HXX
# define URBI_SDK_HXX

#include <object/global.hh>
#include <urbi/sdk.hh>

namespace urbi
{
  rObject call(rObject self,
               libport::Symbol msg,
               rObject arg1,
               rObject arg2,
               rObject arg3,
               rObject arg4,
               rObject arg5)
  {
    return object::urbi_call(self, msg, arg1, arg2, arg3, arg4, arg5);
  }

  rObject global()
  {
    return object::global_class;
  }
}

#endif
