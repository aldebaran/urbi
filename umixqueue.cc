/*! \file umixqueue.cc
 *******************************************************************************

 File: umixqueue.cc\n
 Implementation of the UMixQueue class.

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

#include <stdlib.h>
#include <string.h>

#include "uqueue.h"
#include "umixqueue.h"
#include "userver.h"

//! UMixQueue constructor.
/*! UMixQueue constructor simply calls the UQueue constructor with the same
    behavior.
*/
UMixQueue::UMixQueue  ( int minBufferSize,
                        int maxBufferSize,
                        int adaptive) :
  UQueue (minBufferSize,
          maxBufferSize,
          adaptive)
{
  ADDOBJ(UMixQueue);
  FREEOBJ(UQueue);
}

//! UMixQueue destructor.
UMixQueue::~UMixQueue() 
{
  ADDOBJ(UQueue);
  FREEOBJ(UMixQueue);
}


//! Push the buffer in mixed mode.
/*! Push the buffer at the beginning of the queue, over existing data by
    adding the buffer value to the data value. Used for sound mixing for 
    wav buffers.
*/ 
UErrorValue
UMixQueue::pushMix(const ubyte *buffer, int length, UBlend blend) {
 
  int cursor = start_;
  short current;
  short current2;

  int tmpvalue;
 
  if (length > dataSize_) {
   
    int splitSize = dataSize_;
    pushMix(buffer,dataSize_,blend);
   
    return (push(buffer+splitSize, length - splitSize));    
  }

  for (int i=0;i<length/2;i++) {
   
    memcpy(&current,  buffer_ + cursor,sizeof(short));
    memcpy(&current2, buffer  + 2*i, sizeof(short));

    tmpvalue = 0;
    if (blend == UMIX) tmpvalue = ((int)current + (int)current2)/2;
    if (blend == UADD) tmpvalue = (int)current + (int)current2;
    if (blend == UNORMAL) tmpvalue = (int)current2;
        
    if (tmpvalue >  32767) tmpvalue = 32767;
    if (tmpvalue < -32767) tmpvalue = -32767;
    current = (short)tmpvalue;

    memcpy(buffer_ + cursor, &current , sizeof(short));      
   
    cursor = cursor + 2;
    if (cursor >= bufferSize_) cursor = cursor - bufferSize_;
  }
  return (USUCCESS);
}
