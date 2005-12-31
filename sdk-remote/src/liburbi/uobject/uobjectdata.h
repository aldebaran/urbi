/*! \file uobjectdata.h
 *******************************************************************************

 File: uobjectdata.h\n
 Definition of the UObjectData class and necessary related classes.

 This file is part of LIBURBI\n
 (c) Jean-Christophe Baillie, 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#ifndef UOBJECTDATA_H_DEFINED
#define UOBJECTDATA_H_DEFINED

#include <uclient.h>

namespace URBI {
 
  class UObject;

  extern string serverIP;
  extern UObject* lastUObject;

  //! Main UObjectData class definition for module architecture
  class UObjectData
  {
  public:
    
    UObjectData(UObject *);
    ~UObjectData();

  private:

    UObject *object;
  };

  
} // namespace URBI

#endif

