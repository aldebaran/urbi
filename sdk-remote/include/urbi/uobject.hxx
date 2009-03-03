/// \file urbi/uobject.hxx

#ifndef URBI_UOBJECT_HXX
# define URBI_UOBJECT_HXX

namespace urbi
{

  inline
  UObjectHub*
  getUObjectHub(const std::string& n)
  {
    return baseURBIStarterHub::find(n);
  }

  inline
  UObject*
  getUObject(const std::string& n)
  {
    return baseURBIStarter::find(n);
  }

}

#endif // !URBI_UOBJECT_HXX
