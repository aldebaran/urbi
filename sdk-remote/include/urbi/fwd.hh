/// \file   urbi/fwd.hh
/// \brief  Forward declarations.

#ifndef URBI_FWD_HH
# define URBI_FWD_HH

# include <list>
# include <string>

# include <urbi/export.hh>

namespace urbi
{
  class UAbstractClient;
  class UBinary;
  class UCallbackList;
  class UCallbackWrapper;
  class UClient;
  class UGenericCallback;
  class UImage;
  class UList;
  class UMessage;
  class UObject;
  class UObjectData;
  class UObjectHub;
  class USound;
  class UTimerCallback;
  class UValue;
  class UVar;
  class UVardata;

  typedef std::list<UObject*> UObjectList;

  URBI_SDK_API std::string getClientConnectionID(const UAbstractClient* cli);
  URBI_SDK_API UClient* getDefaultClient();

  class UVariable;
  class UContext;
  namespace impl
  {
    class UContextImpl;
    class UObjectImpl;
    class UVarImpl;
    class UGenericCallbackImpl;
  }

  class baseURBIStarter;
  class baseURBIStarterHub;
};

#endif //! URBI_FWD_HH
