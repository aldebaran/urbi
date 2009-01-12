#include <urbi/ustarter.hh>

namespace urbi
{
  baseURBIStarter::list_type&
  object_list()
  {
    static baseURBIStarter::list_type list;
    return list;
  }

  baseURBIStarterHub::list_type&
  objecthub_list()
  {
    static baseURBIStarterHub::list_type list;
    return list;
  }
}
