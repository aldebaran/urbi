/// \file urbi/uobject.hxx

#ifndef URBI_UOBJECT_HXX
# define URBI_UOBJECT_HXX

namespace urbi
{

  UObjectHub*
  getUObjectHub(const std::string& n)
  {
    return baseURBIStarterHub::find(n);
  }

  UObject*
  getUObject(const std::string& n)
  {
    return baseURBIStarter::find(n);
  }

}

#endif // !URBI_UOBJECT_HXX
