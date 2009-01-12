#include <urbi/ustarter.hh>

namespace urbi
{
  baseURBIStarter::list_type&
  object_list()
  {
    static baseURBIStarter::list_type* list = 0;
    if (!list)
      list = new baseURBIStarter::list_type;
    return *list;
  }
  baseURBIStarterHub::list_type&
  objecthub_list()
  {
    static baseURBIStarterHub::list_type* list = 0;
    if (!list)
      list = new baseURBIStarterHub::list_type;
    return *list;
  }
}
