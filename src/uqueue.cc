/*! \file uqueue.cc
 *******************************************************************************

 File: uqueue.cc\n
 Implementation of the UQueue class.

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

#include <cstdlib>
#include "libport/cstring"

#include "uqueue.hh"
#include "userver.hh"

//! UQueue constructor.
/*! UQueue implements a dynamic circular FIFO buffer.
    You can specify the following parameters:

    \param minBufferSize is the initial size of the buffer. If the buffer is
	   shrinked (adaptive buffer), its size will be at least minBufferSize.
	   The default value if not specified is 4096 octets.
    \param maxBufferSize is the maximal size of the buffer. The buffer size is
	   increased when one tries to push data that is too big for the current
	   buffer size. However, this dynamic behavior is limited to
	   maxBufferSize in any case. If the buffer can still not hold the
	   pushed data, the push() function returns UFAIL.
	   If set to zero, the limit is infinite.
	   The default value is equal to minBufferSize.
    \param adaptive the default behavior of the UQueue is to increase the size
	   of the internal buffer if one tries to hold more data that what the
	   current size allows (up to maxBufferSize, as we said). However, if
	   the buffer is adaptive, it will also be able to shrink the buffer.
	   This is very useful to allow the UQueue to grow momentarily to hold
	   a large amount of data and then recover memory after the boost, if
	   this data size is not the usual functioning load.
	   If adaptive is non null, the behavior is the following: the UQueue
	   counts the number of calls to the pop() function. Each time this
	   number goes above 'adaptive', the UQueue will resize to the maximum
	   data size it has held in the past 'adaptive' calls to pop(), and
	   minBufferSize at least. So, 'adaptive' is a time windows that is
	   periodicaly checked to shink the buffer if necessary.
	   Note that the buffer will shrink only if there is a minimum of 20%
	   size difference, to avoid time consuming reallocs for nothing.
	   We recommand a default value of 100 for the adaptive value.

    When exiting, the internal UError static variable can have the following
    values:

    - USUCCESS: success
    - UFAIL   : memory allocation failed.
*/
UQueue::UQueue  (int minBufferSize,
		  int maxBufferSize,
		  int adaptive)
{
  ADDOBJ(UQueue);

  if (minBufferSize != 0)
    minBufferSize_ = minBufferSize;
  else
    minBufferSize_ = UQueue::INITIAL_BUFFER_SIZE;

  if (maxBufferSize == -1)
    maxBufferSize_ = minBufferSize_;
  else
    maxBufferSize_ = maxBufferSize;

  adaptive_        = adaptive;

  outputBuffer_    = 0;

  // Internal buffer initialization.
  buffer_ = (ubyte*) malloc(minBufferSize_);
  if (buffer_== 0)
  {
    UError = UFAIL;
    return;
  }
  ADDMEM(minBufferSize_);

  bufferSize_      = minBufferSize_;
  start_           = 0;
  end_             = 0;
  dataSize_        = 0;

  // adaptive initialization
  nbPopCall_       = 0;
  topDataSize_     = 0;

  // Output buffer initialization (the size can grow later)
  outputBuffer_ = (ubyte*) malloc(UQueue::INITIAL_BUFFER_SIZE);
  if (outputBuffer_== 0)
  {
    free (buffer_);
    FREEMEM(bufferSize_);
    buffer_ = 0;
    UError = UFAIL;
    return;
  }
  ADDMEM(UQueue::INITIAL_BUFFER_SIZE);
  outputBufferSize_ = UQueue::INITIAL_BUFFER_SIZE;
  topOutputSize_ = 0;

  // mark the beginning of the buffer
  mark_ = 0;
  locked_ = false;

  UError = USUCCESS;
}

//! UQueue destructor.
UQueue::~UQueue()
{
  FREEOBJ(UQueue);

  if (buffer_)
  {
    free (buffer_);
    FREEMEM(bufferSize_);
  }

  if (outputBuffer_)
  {
    free (outputBuffer_);
    FREEMEM(outputBufferSize_);
  }
}

//! Clear the queue
void
UQueue::clear()
{
  start_    = 0;
  end_      = 0;
  dataSize_ = 0;
}

//! Set a mark to be able to revert to this position
void
UQueue::mark()
{
  mark_ = end_;
  locked_ = false;
}

//! Revert the buffer to the marked position.
void
UQueue::revert()
{
  end_ = mark_;
  locked_ = true;
}

//! Pushes a buffer into the queue..
/*! This function tries to store the buffer in the queue, and tries to extend
    to buffer size when the buffer does not fit and if it is possible.

    \param buffer the buffer to push
    \param length the length of the buffer
    \return
	    - USUCCESS: successful
	    - UFAIL   : could not push the buffer. The queue is not changed.
*/
UErrorValue
UQueue::push (const ubyte *buffer, int length)
{
  int bfs;
  int newSize;

  bfs = bufferFreeSpace();

  if (bfs < length) { // Is the internal buffer big enough?

    // No. Check if the internal buffer can be extended.
    newSize = bufferSize_ + (length - bfs);

    if (newSize > maxBufferSize_ && maxBufferSize_ != 0)
    {
      return UFAIL;
    }
    else
    {
      // Yes, the internal buffer can be extended.

      ubyte *newBuffer;

      // Calculate the required size + 10%, if it fits.
      newSize = (int)(1.10 * newSize);
      if (newSize%2!=0) newSize++; // hack for short alignment...
      if ((newSize > maxBufferSize_) && (maxBufferSize_ != 0))
	newSize = maxBufferSize_;

      // Realloc the internal buffer
      newBuffer = (ubyte*)realloc((void*)buffer_, newSize);
      if (newBuffer == 0) return UMEMORYFAIL; // not enough memory.
      ADDMEM(newSize - bufferSize_);

      buffer_ = newBuffer;
      if ((end_ < start_) || (bfs == 0 ))
      {
	// Translate the rightside of the old internal buffer.

	memmove(buffer_ + start_ + (newSize - bufferSize_),
		buffer_ + start_,
		bufferSize_ - start_);
	start_ += (newSize - bufferSize_);
      }

      bufferSize_ = newSize;
    }
  }

  if (bufferSize_ - end_ >= length)
  {
    // Do we have to split 'buffer'?
    // No need to split.

    memcpy(buffer_ + end_, buffer, length);
    end_ += length;
    if (end_ == bufferSize_) end_ = 0; // loop the circular geometry.

  }
  else
  {
    // Split 'buffer' to fit in the internal circular buffer.

    memcpy(buffer_ + end_, buffer, bufferSize_ - end_);
    memcpy(buffer_, buffer + (bufferSize_ - end_), length-(bufferSize_ - end_));
    end_ = length - (bufferSize_ - end_);
  }

  dataSize_ += length;

  return USUCCESS;
}



//! Pops 'length' bytes out of the Queue
/*! Pops 'length' bytes.
    The ubyte* pointer returned is not permanent. You have to make a copy of the
    data pointed if you want to keep it. Any call to a member function of UQueue
    might alter it later.

    \param length the length requested.
    \return a pointer to the the data popped or 0 in case of error.
*/
ubyte*
UQueue::pop (int length)
{
  int toPop;    // nb of bytes to send

  // Actual size of the data to pop.
  toPop = length;
  if (toPop > dataSize_) return 0; // Not enough data to pop 'length'
  if (toPop == 0)
    return buffer_ + start_;  // Pops nothing but gets a pointer to the
			      // beginning of the buffer.
  // Adaptive shrinking behavior
  if (adaptive_)
  {
    nbPopCall_++;
    if (dataSize_ > topDataSize_) topDataSize_ = dataSize_;
    if (toPop > topOutputSize_)   topOutputSize_ = toPop;

    if (nbPopCall_ > adaptive_ ) { // time out

      nbPopCall_ = 0; // reset

      if (topOutputSize_ < (int)(outputBufferSize_ * 0.8))
      {
	// We shrink the output buffer to the new size: topOutputSize_ + 10%
	topOutputSize_ = (int)(topOutputSize_ * 1.1);

	ubyte* newOutputBuffer = (ubyte*)realloc(outputBuffer_, topOutputSize_);
	if (newOutputBuffer != 0)
	{
	  FREEMEM(outputBufferSize_ - topOutputSize_);
	  outputBuffer_ = newOutputBuffer;
	  outputBufferSize_ = topOutputSize_;
	}
      }

      if (topDataSize_ < (int) (bufferSize_ * 0.8))
      {
	// We shrink the buffer to the new size: topDataSize_ + 10% (if it fits)
	topDataSize_ = (int) (topDataSize_ * 1.1);
	if ((topDataSize_ > maxBufferSize_) && (maxBufferSize_ !=0))
	  topDataSize_ = maxBufferSize_;

	if (end_ < start_)
	{
	  // The data is splitted
	  memmove(buffer_ + start_ - (bufferSize_ - topDataSize_),
		  buffer_ + start_,
		  bufferSize_ - start_);
	  start_ = start_ - (bufferSize_ - topDataSize_);
	}
	else
	{
	  // The data is contiguous
	  memmove(buffer_,
		  buffer_ + start_,
		  dataSize_);
	  start_ = 0;
	  end_   = dataSize_;
	  // the case end_ == bufferSize_ is handled below.
	}

	ubyte* newBuffer = (ubyte*) realloc (buffer_, topDataSize_);
	if (newBuffer != 0)
	{
	  FREEMEM(bufferSize_ - topDataSize_);
	  buffer_ = newBuffer;
	  bufferSize_ = topDataSize_;
	  if (end_ == bufferSize_ ) end_ =0; // loop the circular geometry.
	}
	// else... well it should never come to this else anyway.
      }

      topDataSize_   = 0;
      topOutputSize_ = 0;
    }
  }

  if (bufferSize_ - start_ >= toPop) { // Is the packet continuous across the
					// the internal buffer?
    // yes, the packet is continuous in the internal buffer

    int tmp_index = start_;
    start_ += toPop;
    if (start_ == bufferSize_) start_ = 0; // loop the circular geometry.
    dataSize_ -= toPop;

    return buffer_ + tmp_index;

  }
  else
  {
    // no, the packet spans then end and the beginning of the buffer
    // and it must be reconstructed.

    // Is the temporary internal outputBuffer large enough?
    if (outputBufferSize_ < toPop)
    {
      // Realloc the internal outputBuffer
      int theNewSize = (int)(toPop*1.10);
      if (theNewSize%2!=0) theNewSize++;
      ubyte* newOutputBuffer = (ubyte*)realloc((void*)outputBuffer_,
						theNewSize );
      if (newOutputBuffer == 0) return 0; // not enough memory.
      ADDMEM(theNewSize - outputBufferSize_);
      outputBuffer_ = newOutputBuffer;
      outputBufferSize_ = theNewSize;
    }

    memcpy(outputBuffer_,
	   buffer_ + start_,
	   bufferSize_ - start_);

    memcpy(outputBuffer_ + (bufferSize_ - start_ ),
	   buffer_,
	   toPop - (bufferSize_ - start_));

    start_ = toPop - (bufferSize_ - start_);
    dataSize_ -= toPop;

    return outputBuffer_;
  }
}

//! Pops at most 'length' bytes out of the Queue
/*! Pops at most 'length' bytes. The actual size is returned in 'length'.
    The ubyte* pointer returned is not permanent. You have to make a copy of the
    data pointed if you want to keep it. Any call to a member function of UQueue
    might alter it later.

    This method is called "fast" because is will not use the temporary
    outputBuffer if the requested data is half at the end and half at the
    beginning of the buffer. It will return only the part at the end of the
    buffer and return the actual size of data popped. You have to check if this
    value is equal to the requested length. If it is not, a second call to
    fastPop will give you the rest of the buffer, with a different pointer.
    This function is useful if you plan to pop huge amount of data which will
    probably span over the end of the buffer and which would, with a simple
    call to 'pop', result in a huge memory replication. In other cases, prefer
    'pop', which is most of the time as efficient as fastPop and much more
    convenient to use.

    \param length the length requested. Contains the actual length popped.
    \return a pointer to the the data popped or 0 in case of error.
*/
ubyte*
UQueue::fastPop (int &length)
{
  return pop((length > bufferSize_ - start_) ?
	      (length = bufferSize_ - start_) :
	      length );
}


//! Simulates the pop of 'length' bytes out of the Queue
/*! Behave like pop but simulate the effect. This is useful if one wants to try
    something with the popped buffer and check the validity before actually
    popping the data.
    It is also typically used when trying to send bytes in a connection and
    it is not known in advance how many bytes will be effectively sent.

    \param length the length requested.
    \return a pointer to the the data popped or 0 in case of error.
*/
ubyte*
UQueue::virtualPop (int length)
{
  int toPop;    // nb of bytes to send

  // Actual size of the data to pop.
  toPop = length;
  if (toPop > dataSize_) return 0; // Not enough data to pop 'length'
  if (toPop == 0)
    return buffer_ + start_;  // Pops nothing but gets a pointer to the
			      // beginning of the buffer.

  if (bufferSize_ - start_ >= toPop) { // Is the packet continuous across the
					// the internal buffer?
    // yes, the packet is continuous in the internal buffer

    int tmp_index = start_;
    //start_ += toPop;
    //if (start_ == bufferSize_) start_ = 0; // loop the circular geometry.
    //dataSize_ -= toPop;

    return buffer_ + tmp_index;

  }
  else
  {
    // no, the packet spans then end and the beginning of the buffer
    // and it must be reconstructed.

    // Is the temporary internal outputBuffer large enough?
    if (outputBufferSize_ < toPop)
    {
      // Realloc the internal outputBuffer
      int theNewSize = (int)(toPop*1.10);
      if (theNewSize%2!=0) theNewSize++;
      ubyte* newOutputBuffer = (ubyte*)realloc((void*)outputBuffer_,
						theNewSize );
      if (newOutputBuffer == 0)
	// not enough memory.
	return 0;
      ADDMEM(theNewSize - outputBufferSize_);
      outputBuffer_ = newOutputBuffer;
      outputBufferSize_ = theNewSize;
    }

    memcpy(outputBuffer_,
	   buffer_ + start_,
	   bufferSize_ - start_);

    memcpy(outputBuffer_ + (bufferSize_ - start_),
	   buffer_,
	   toPop - (bufferSize_ - start_));

    //start_ = toPop - (bufferSize_ - start_);
    //dataSize_ -= toPop;

    return outputBuffer_;
  }
}
