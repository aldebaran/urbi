/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file   urbi/fwd.hh
/// \brief  Forward declarations.

#ifndef URBI_FWD_HH
# define URBI_FWD_HH

# include <list>
# include <string>
# include <boost/shared_ptr.hpp>
# include <urbi/export.hh>

namespace urbi
{
  namespace impl
  {
    class RemoteUContextImpl;
    class RemoteUVarImpl;
    class UContextImpl;
    class UGenericCallbackImpl;
    class UObjectImpl;
    class UVarImpl;
  }

  class InputPort;
  class UAbstractClient;
  class UBinary;
  class UCallbackInterface;
  class UCallbackList;
  class UCallbackWrapper;
  class UClient;
  class UContext;
  class UEvent;
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
  class UVariable;
  class baseURBIStarter;
  class baseURBIStarterHub;

  typedef std::list<UObject*> UObjectList;

  URBI_SDK_API std::string getClientConnectionID(const UAbstractClient* c);
  URBI_SDK_API UClient* getDefaultClient();

  typedef boost::shared_ptr<std::string> TimerHandle;
};

#endif //! URBI_FWD_HH
