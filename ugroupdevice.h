/*! \file ugroupdevice.h
 *******************************************************************************

 File: ugroupdevice.h\n
 Definition of the UGroupDevice class.

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

#ifndef UGROUPDEVICE_H_DEFINED
#define UGROUPDEVICE_H_DEFINED

#include "ustring.h"
#include "udevice.h"
#include "uvariablename.h"

class UGroupDevice : public UDevice {
 public:
  UGroupDevice(const UString &name);
  virtual void        notifyWrite     ( const UVariable *variable);
  virtual void        notifyRead      ( const UVariable *variable);
  virtual UValue*     evalFunction    ( UCommand *command,
                                        UConnection *connection,                                       
                                        const char *method,
                                        UNamedParameters *parameters);
  UValue * list( UVariableName *variable);

};

#endif
