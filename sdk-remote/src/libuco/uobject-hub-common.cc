/// \file libuco/uobject-hub-common.cc

#include <urbi/uobject.hh>

//! Global definition of the starterlist
namespace urbi
{
  /*-------------.
  | UObjectHub.  |
  `-------------*/

  //! UObjectHub constructor.
  UObjectHub::UObjectHub(const std::string& s)
    : name(s)
  {
  }

  int
  UObjectHub::updateGlobal()
  {
    for (UObjectList::iterator i = members.begin(); i != members.end(); ++i)
      (*i)->update();
    update();
    return 0;
  }

  void
  UObjectHub::addMember(UObject* obj)
  {
    members.push_back(obj);
  }

  void
  UObjectHub::delMember(UObject* obj)
  {
    members.remove (obj);
  }

  UObjectList*
  UObjectHub::getSubClass(const std::string& subclass)
  {
    UObjectList* res = new UObjectList();
    for (UObjectList::iterator i = members.begin(); i != members.end(); ++i)
      if ((*i)->classname == subclass)
	res->push_back(*i);

    return res;
  }


  /*-------------------------.
  | Freestanding functions.  |
  `-------------------------*/

  //! retrieve a UObjectHub based on its name
  UObjectHub*
  getUObjectHub(const std::string& name)
  {
    for (baseURBIStarterHub::list_type::iterator
           i = baseURBIStarterHub::list.begin(),
           i_end = baseURBIStarterHub::list.end();
	 i != i_end; ++i)
      if ((*i)->name == name)
	return (*i)->getUObjectHub();
    return 0;
  }

  //! Retrieve a UObject based on its name.
  UObject*
  getUObject(const std::string& name)
  {
    for (baseURBIStarter::list_type::iterator
           i = baseURBIStarter::list.begin(),
           i_end = baseURBIStarter::list.end();
	 i != i_end; ++i)
      if ((*i)->name == name)
	return (*i)->getUObject();
    return 0;
  }


} // namespace urbi
