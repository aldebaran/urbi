/// \file urbi/uobject.hxx

#ifndef URBI_UOBJECT_HXX
# define URBI_UOBJECT_HXX

namespace urbi
{

  inline
  bool
  isPluginMode()
  {
    return getRunningMode() == MODE_PLUGIN;
  }

  inline
  bool
  isRemoteMode()
  {
    return getRunningMode() == MODE_REMOTE;
  }

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
