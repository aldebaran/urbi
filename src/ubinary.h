/*! \file ubinary.h
 *******************************************************************************

 File: ubinary.h\n
 Definition of the UBinary class.

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

#ifndef UBINARY_H_DEFINED
#define UBINARY_H_DEFINED

#include "unamedparameters.h"

// *****************************************************************************
//! Contains binary data, an the binary buffer in particular.
class UBinary
{
public:

  UBinary(int bufferSize, UNamedParameters *parameters);
  ~UBinary();

  void     print(); 

  ubyte             *buffer;       ///< The buffer used to store the binary data
  int               bufferSize;    ///< The size of the buffer
  UNamedParameters  *parameters;   ///< The optional parameters of the binary
};

#endif
