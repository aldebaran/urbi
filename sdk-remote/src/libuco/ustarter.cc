#include <urbi/ustarter.hh>

namespace urbi
{

  /*------------------.
  | baseURBIStarter.  |
  `------------------*/

  void
  baseURBIStarter::copy_(UObject* uso)
  {
    getUObject()->members.push_back(uso);
    uso->derived = true;
    uso->classname = getUObject()->classname;
    if (uso->autogroup)
      uso->addAutoGroup();
  }

  baseURBIStarter::list_type&
  baseURBIStarter::list()
  {
    static list_type list;
    return list;
  }


  UObject*
  baseURBIStarter::find(const std::string& name)
  {
    for (list_type::iterator i = list().begin(), i_end = list().end();
	 i != i_end; ++i)
      if ((*i)->name == name)
	return (*i)->getUObject();
    return 0;
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

  UObjectHub*
  baseURBIStarterHub::find(const std::string& name)
  {
    for (list_type::iterator i = list().begin(), i_end = list().end();
	 i != i_end; ++i)
      if ((*i)->name == name)
	return (*i)->getUObjectHub();
    return 0;
  }

}
