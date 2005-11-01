/*! \file udevice.h
 *******************************************************************************

 File: udevice.h\n
 Definition of the UDevice class.

 This file is part of 
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UDEVICE_H_DEFINED
#define UDEVICE_H_DEFINED

//#include "userver.h"
#include "utypes.h"

class UCommand;
class UVariable;
class UValue;
class UConnection;
class UNamedParameters;

//! UDevice class handles a virtual device.
/*! This class stores every robot independant information for a device and 
    handles the trajectory mechanism used in URBI.

    It must be overloaded to match a particular robot device.
*/
class UDevice
{
 public:
	
  UDevice  ( const char    *device,
             const char    *detail,
             double         val_rangemin,
             double         val_rangemax,            
             const char    *val_unit,
             UDataType      val_dataType,
             bool           val_autoUpdate,
             bool           val_notifyWrite,
             bool           val_notifyRead = false);

  virtual ~UDevice ();

  virtual void        notifyWrite     ( const UVariable *variable);
  virtual void        notifyRead      ( const UVariable *variable);
  virtual UValue*     evalFunction    ( UCommand *command,
                                        UConnection *connection,                                       
                                        const char *method,
                                        UNamedParameters *parameters);

  UVariable*          getVariable( const char* var );
  UValue*             getValue   ( const char* var );

  UString             *device;  ///< device name
  UString             *detail;  ///< details about the device

  UVariable           *device_val; ///< ref to the variable device.val
  UVariable           *device_load; ///< ref to the variable device.val
};

#endif
