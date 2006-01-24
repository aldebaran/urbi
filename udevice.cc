/*! \file udevice.cc
 *******************************************************************************

 File: udevice.cc\n
 Implementation of the UDevice class.

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

#include <stdio.h>
#include "udevice.h"
#include "userver.h"
	
//! UDevice constructor.
/*! The first parameters are obvious. The last three needs more explanation:

    - val_autoUpdate: if true, the kernel will do the junction between the 
      intermediary value used during the execution of the command tree and
      the actual device.val value automatically. Otherwise, it has to be done
      by the programmer. The use of this is for joint devices for example: 
      you want to set the value of the device.val based on the sensor output and
      not the theoretical trajectory calculcation from the kernel. In that case,
      autoUpdate will be false: you will set the correct value during the sensor
      update. For other devices, like LEDs, there is no sensor associated. The 
      theoretical value is always the actual value, so autoUpdate will be true.

    - val_notifyWrite: this applies for the device.val variable and requests that
      the device is notified when the variable is modified or not. See
      notifyWrite() for more details.

    - val_notifyRead: request a call to notifyRead() before any access to
      device.val. This can be useful for low bandwidth sensor buses.
 */
UDevice::UDevice(const char    *device,
                 const char    *detail,
                 double         val_rangemin,
                 double         val_rangemax,            
                 const char    *val_unit,
                 UDataType      val_dataType,
                 bool           val_autoUpdate,
                 bool           val_notifyWrite,
                 bool           val_notifyRead) 
{	
  this->device   = new UString (device);
  this->detail   = new UString (detail);
 
  // Store it in the device table

  ::urbiserver->devicetab[this->device->str()] = this;

  // Associate a value and a corresponding device.val variable

  UValue* value = new UValue(); // no test on value!=0. If it fail, the server will crash
                        // anyway...  

  value->dataType = val_dataType;

  // device_val and device_load are quick references to the underlying
  // variables, which can be used to quicken the branching in 
  // notifyWrite() or notifyRead().
  device_val  = new UVariable(device, "val", value,
                              val_notifyWrite, 
                              val_notifyRead,
                              val_autoUpdate);
  device_val->rangemin = val_rangemin;
  device_val->rangemax = val_rangemax;
  device_val->unit     = new UString (val_unit);
  device_val->modified = true; 
  device_load = new UVariable(device, "load", 1.0, true);

  // create MAINDEVICE.device[k]  
  char tmpname[512];
  snprintf(tmpname,512,"%s.device__%d",MAINDEVICE,::urbiserver->devicetab.size()-1);    
  ::urbiserver->addAlias(tmpname, device_val->varname->str());
  snprintf(tmpname,512,"%s.devicename__%d",MAINDEVICE,::urbiserver->devicetab.size()-1); 
  new UVariable(tmpname, device);

}

//! UDevice destructor.
UDevice::~UDevice() 
{
  if (device) delete device;
  if (detail) delete detail;
  if (device_val) delete device_val;
  if (device_load) delete device_load;
}

//! UDevice variable modificatin notification 
/*! When a variable associated to the device and with the "notifyWrite"
    flag on is modified, this function is called. A simple test on the
    pointer value in "variable" can be done and specific action can be
    undertaken. 

    This virtual function must be redefined for your device type.
 */
void
UDevice::notifyWrite(const UVariable *variable)
{
}

//! UDevice variable read attempt notification 
/*! When a variable associated to the device and with the "notifyRead"
    flag on is about to be read, this function is called. A simple test on the
    pointer value in "variable" can be done and specific action can be
    undertaken to update the value.

    This virtual function must be redefined for your device type.
 */
void
UDevice::notifyRead(const UVariable *variable)
{
}

//! Returns the device variable device.var, if it exists
UVariable*  
UDevice::getVariable(const char* var)
{
  return( ::urbiserver->getVariable(device->str(), var) );
}

//! UDevice property UValue get
/*! This function calls updateVariable before returning the result.
    It is safe to use it for any device property that needs an update
    from the robot-specific part, typically device.val or other
    device readable sensor information
*/
UValue*  
UDevice::getValue(const char* var)
{
  UVariable *tmpVar = ::urbiserver->getVariable(device->str(), var);
  if (tmpVar) {
    notifyRead(tmpVar);
    return (tmpVar->value);
  }
  else
    return(0);
}


//! Eval a custom device specific function. 
/*! This function handles some robot-specific functions that you might
    want to implement for your robot. The kernel will call it each time
    it encounters a function call with a device prefix that matches 
    the corresponding UDevice.

    Typical example is speaker.play(file) which plays the sound specified
    in the file. For obvious reasons, this is not the job of the kernel,
    so you will have to intercept it and handle it on the robot-specific
    side.
    The function returns a UValue*. If you just want to return a double,
    do this:

    return( new UValue(mydouble) );

    Don't worry about memory allocation. The kernel handles it.

    If you want to return "void", do this:

    return ( new UValue() );

    If you want to signal an error, or if you don't known the function,
    do this:

    return (0);    

    The command & connection are given for error outputs purposes.
*/
UValue*
UDevice::evalFunction ( UCommand         *command,
                        UConnection      *connection,                      
                        const char       *method,
                        UNamedParameters *parameters )
{
  return (0); // default setting: do nothing
}

