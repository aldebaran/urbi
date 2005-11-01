/*! \file uqueue.h
 *******************************************************************************

 File: uqueue.h\n
 Definition of the UQueue class.

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

#ifndef UQUEUE_H_DEFINED
#define UQUEUE_H_DEFINED

#include "utypes.h"

/// Dynamic expendable and shrinkable circular FIFO buffer 
/*! UQueue is a FIFO buffer which is both dynamic and circular.

    This FIFO queue can be filled in (push) or partially emptied (pop).

    * Dynamic means that if the buffer is not big enough to push new data in, it
    will be expanded, up to a given limit (maxBufferSize). After that limit, 
    a buffer overflow error will occur and the push will return UFAIL.
    If the adaptive option is used, the buffer will shrink again if its size
    is too big, according to the recent past of data size it has holded. This
    option is fully described in the constructor. It is implemented in the pop()
    function.

    * Circular means that if the data pushed cannot hold at the end of the 
    buffer, it will be splitted and looped at the beginning, thus avoiding any 
    memory space loss. In other words, the topology of the buffer is such that 
    the end connects the beginning.
 */
class UQueue 
{
public:
	
  UQueue  (int minBufferSize = 0,
           int maxBufferSize = -1,
           int adaptive = 0);

  virtual ~UQueue ();

  UErrorValue         push              (const char *s);
  UErrorValue         push              (const ubyte *buffer, int length);

  ubyte*              pop               (int length);
  ubyte*              virtualPop        (int length);
  ubyte*              fastPop           (int &length);

  void                clear             ();
  
  int                 bufferFreeSpace   ();
  int                 bufferMaxFreeSpace();
  int                 dataSize          ();

  UErrorValue         UError;///< err code for the constructor

protected:

  static const int INITIAL_BUFFER_SIZE = 4096; ///< initial size of the
                                               ///< output buffer used by pop.

  ubyte          *buffer_;       ///< queue internal buffer (circular).
  ubyte          *outputBuffer_; ///< buffer used by pop to return it's value.

  int            bufferSize_;    ///< current internal buffer size.
  int            outputBufferSize_; ///< size of the output buffer used by pop.
  int            start_;         ///< internal buffer start offset.
  int            end_;           ///< internal buffer end offset.
  int            dataSize_;      ///< size of the data inside the buffer.

  int            minBufferSize_; ///< Stores the initial size of the internal 
                                 ///< buffer.
  int            maxBufferSize_; ///< This is the maximum size the dynamical 
                                 ///< internal buffer is allowed to grow.
  int            adaptive_;      ///< This is the size of the time window used
                                 ///< by the adaptive algorithm.
  int            nbPopCall_;     ///< nb calls to the pop() function.
  int            topDataSize_;   ///< maximal data size in the adaptive time 
                                 ///< windows. Used to shrink the buffer.
  int            topOutputSize_; ///< maximal data size outputed in the time
                                 ///< windows. Used to shrink outputBuffer.
};


//! Pushes a string into the queue. The zero ending character is ignored.
/*! The string is converted into a ubyte* buffer and the function calls the
    push(ubyte*,int) function.

    \param s the string to send
    \return 
            - USUCCESS: successful
            - UFAIL   : could not push the string.
    \sa push(const ubyte*,int)
*/
inline UErrorValue      
UQueue::push (const char *s)
{
  return  push((const ubyte*)s,strlen(s));
}

//! returns the available free space in the buffer.
inline int 
UQueue::bufferFreeSpace()
{
  return( bufferSize_ - dataSize_ );
}

//! returns the max available free space in the buffer.
inline int 
UQueue::bufferMaxFreeSpace()
{
  return( maxBufferSize_ - dataSize_ );
}

//! returns the size of the data in the buffer
inline int 
UQueue::dataSize()
{
  return( dataSize_ );
}


#endif
