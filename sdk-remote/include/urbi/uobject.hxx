/// \file urbi/uobject.hxx

#ifndef URBI_UOBJECT_HXX
# define URBI_UOBJECT_HXX

namespace urbi
{

  /*----------.
  | UObject.  |
  `----------*/

  inline
  int
  UObject::update()
  {
    return 0;
  }

  inline
  int
  UObject::voidfun()
  {
    return 0;
  }

  /*-------------.
  | Standalone.  |
  `-------------*/

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

}

#endif // !URBI_UOBJECT_HXX
