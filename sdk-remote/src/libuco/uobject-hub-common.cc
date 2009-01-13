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

} // namespace urbi
