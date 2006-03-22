/*! \file ufunction.h
 *******************************************************************************

 File: ufunction.h\n
 Definition of useful types in the URBI server kernel.

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

#ifndef UFUNCTION_H_DEFINED
#define UFUNCTION_H_DEFINED

#include <string.h>
#include <stdlib.h>

class UString;
class UNamedParameters;
class UCommand;

//! UFunction is used to handle functions in the URBI server 
class UFunction {
public:

  UFunction(UString *funname,
            UNamedParameters *parameters,
            UCommand *command);
  ~UFunction();

  UString*   name();
  int        nbparam();
  UCommand*  cmdcopy(UString *_tag = 0, 
                     UNamedParameters *_flags = 0);
  UString          *funname; ///< name of the function
  UNamedParameters *parameters; ///< parameters of the function
  UCommand         *command; ///< body of the function
};

#endif
