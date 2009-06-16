#include <libport/foreach.hh>
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
    foreach (baseURBIStarter* s, list())
      if (s->name == name)
	return s->getUObject();
    return 0;
  }

  void
  baseURBIStarter::init()
  {
    foreach (baseURBIStarter* s, list())
      s->init(s->name);
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
