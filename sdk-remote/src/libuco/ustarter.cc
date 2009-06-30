#include <libport/foreach.hh>
#include <urbi/ustarter.hh>

namespace urbi
{

  /*------------------.
  | baseURBIStarter.  |
  `------------------*/


  baseURBIStarter::list_type&
  baseURBIStarter::list()
  {
    static list_type list;
    return list;
  }

  /*---------------------.
  | baseURBIStarterHub.  |
  `---------------------*/

  baseURBIStarterHub::list_type&
  baseURBIStarterHub::list()
  {
    static list_type list;
    return list;
  }

}
