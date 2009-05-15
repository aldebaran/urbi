#include <urbi/uobject.hh>

namespace urbi
{
  void
  UObject::addAutoGroup()
  {
    UJoinGroup(classname + "s");
  }
}
