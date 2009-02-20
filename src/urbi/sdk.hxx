#ifndef URBI_SDK_HXX
# define URBI_SDK_HXX

#include <object/global.hh>
#include <urbi/sdk.hh>

namespace urbi
{
  rObject global()
  {
    return object::global_class;
  }
}

#endif
