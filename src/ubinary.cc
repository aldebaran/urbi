/*! \file ubinary.cc
 *******************************************************************************

 File: ubinary.cc\n
 Implementation of the UBinary class.

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

#include "ubinary.hh"
#include "userver.hh"


// **************************************************************************
//! UBinary constructor.
UBinary::UBinary(int bufferSize, UNamedParameters *parameters)
  : bufferSize (bufferSize),
    buffer ((ubyte*) malloc(bufferSize)), // result tested outside.
    parameters (parameters)
{
  ADDOBJ(UBinary);
  if (buffer)
    ADDMEM(bufferSize);
}

//! UBinary destructor.
UBinary::~UBinary()
{
  FREEOBJ(UBinary);
  delete parameters;
  if (buffer)
  {
    free(buffer);
    FREEMEM(bufferSize);
  }
}

//! Print the binary
/*! This function is for debugging purpose only.
    It is not safe, efficient or crash proof. A better version will come later.
*/
void
UBinary::print()
{
  ::urbiserver->debug("(BIN %d)",bufferSize);
}
