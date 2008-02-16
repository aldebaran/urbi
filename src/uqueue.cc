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

#include "kernel/userver.hh"

#include "uqueue.hh"

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
*/
UQueue::UQueue (size_t minBufferSize,
		size_t maxBufferSize,
		size_t adaptive)
  : minBufferSize_ (minBufferSize
		    ? minBufferSize : static_cast<size_t>(INITIAL_BUFFER_SIZE)),
    maxBufferSize_ (maxBufferSize == static_cast<size_t>(-1)
		    ? minBufferSize_ : maxBufferSize),
    adaptive_(adaptive),
    buffer_(minBufferSize_),
    outputBuffer_(UQueue::INITIAL_BUFFER_SIZE),
    start_(0),
    end_(0),
    dataSize_(0),
    nbPopCall_(0),
    topDataSize_(0),
    topOutputSize_(0),
    mark_(0),
    locked_(false)
{
}

//! UQueue destructor.
UQueue::~UQueue()
{
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
UQueue::push (const ubyte *buffer, size_t length)
{
  size_t bfs = bufferFreeSpace();

  if (bfs < length) // Is the internal buffer big enough?
  {
    // No. Check if the internal buffer can be extended.
    size_t newSize = buffer_.size() + (length - bfs);

    if (newSize > maxBufferSize_ && maxBufferSize_ != 0)
      return UFAIL;
    else
    {
      // Calculate the required size + 10%, if it fits.
      newSize *= 1.1;
      if (newSize % 2 != 0)
	++newSize; // hack for short alignment...
      if (newSize > maxBufferSize_ && maxBufferSize_ != 0)
	newSize = maxBufferSize_;

      // Realloc the internal buffer
      size_t old_size = buffer_.size();
      buffer_.resize(newSize);
      if (end_ < start_ || bfs == 0 )
      {
	// Translate the rightside of the old internal buffer.
	memmove(&buffer_[0] + start_ + newSize - old_size,
		&buffer_[0] + start_,
		old_size - start_);
	start_ += newSize - old_size;
      }
    }
  }

  if (buffer_.size() - end_ >= length)
  {
    // Do we have to split 'buffer'?
    // No need to split.
    memcpy(&buffer_[0] + end_, buffer, length);
    end_ += length;
    if (end_ == buffer_.size())
      end_ = 0; // loop the circular geometry.
  }
  else
  {
    // Split 'buffer' to fit in the internal circular buffer.
    memcpy(&buffer_[0] + end_, buffer, buffer_.size() - end_);
    memcpy(&buffer_[0],
	   buffer + (buffer_.size() - end_),
	   length - (buffer_.size() - end_));
    end_ = length - (buffer_.size() - end_);
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
UQueue::pop (size_t length)
{
  // Actual size of the data to pop.
  size_t toPop = length;
  if (toPop > dataSize_)
    return 0; // Not enough data to pop 'length'

  if (toPop == 0)
    // Pops nothing but gets a pointer to the beginning of the buffer.
    return &buffer_[0] + start_;

  // Adaptive shrinking behavior
  if (adaptive_)
  {
    ++nbPopCall_;
    topDataSize_ = std::max (topDataSize_, dataSize_);
    topOutputSize_ = std::max (topOutputSize_, toPop);

    if (nbPopCall_ > adaptive_ )
    {
      // time out
      nbPopCall_ = 0; // reset

      if (topOutputSize_ < outputBuffer_.size() * 0.8)
	outputBuffer_.resize(topOutputSize_ * 1.1);

      if (topDataSize_ < buffer_.size() * 0.8)
      {
	// We shrink the buffer to the new size: topDataSize_ + 10% (if it fits)
	topDataSize_ *= 1.1;
	if (topDataSize_ > maxBufferSize_ && maxBufferSize_ !=0)
	  topDataSize_ = maxBufferSize_;

	if (end_ < start_)
	{
	  // The data is splitted
	  memmove(&buffer_[0] + start_ - (buffer_.size() - topDataSize_),
		  &buffer_[0] + start_, buffer_.size() - start_);
	  start_ = start_ - (buffer_.size() - topDataSize_);
	}
	else
	{
	  // The data is contiguous
	  memmove(&buffer_[0], &buffer_[0] + start_, dataSize_);
	  start_ = 0;
	  end_   = dataSize_;
	  // the case end_ == buffer_.size() is handled below.
	}

	buffer_.resize(topDataSize_);
	if (end_ == buffer_.size() )
	  end_ =0; // loop the circular geometry.
	// else... well it should never come to this else anyway.
      }

      topDataSize_   = 0;
      topOutputSize_ = 0;
    }
  }

  if (buffer_.size() - start_ >= toPop)
  {
    // Is the packet continuous across the the internal buffer?  yes,
    // the packet is continuous in the internal buffer
    size_t tmp_index = start_;
    start_ += toPop;
    if (start_ == buffer_.size())
      start_ = 0; // loop the circular geometry.
    dataSize_ -= toPop;
    return &buffer_[0] + tmp_index;
  }
  else
  {
    // no, the packet spans then end and the beginning of the buffer
    // and it must be reconstructed.

    // Is the temporary internal outputBuffer large enough?
    if (outputBuffer_.size() < toPop)
      outputBuffer_.resize(toPop * 1.1);

    memcpy(&outputBuffer_[0],
	   &buffer_[0] + start_,
	   buffer_.size() - start_);

    memcpy(&outputBuffer_[0] + (buffer_.size() - start_ ),
	   &buffer_[0],
	   toPop - (buffer_.size() - start_));

    start_ = toPop - (buffer_.size() - start_);
    dataSize_ -= toPop;

    return &outputBuffer_[0];
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
UQueue::fastPop (size_t &length)
{
  return pop((length > buffer_.size() - start_)
	     ? (length = buffer_.size() - start_)
	     : length);
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
UQueue::virtualPop (size_t length)
{
  // Actual size of the data to pop.
  size_t toPop = length;
  if (toPop > dataSize_)
    return 0; // Not enough data to pop 'length'
  if (toPop == 0)
    // Pops nothing but gets a pointer to the beginning of the buffer.
    return &buffer_[0] + start_;

  // Is the packet continuous across the internal buffer?
  if (buffer_.size() - start_ >= toPop)
    // yes, the packet is continuous in the internal buffer
    return &buffer_[0] + start_;
  else
  {
    // no, the packet spans then end and the beginning of the buffer
    // and it must be reconstructed.

    // Is the temporary internal outputBuffer large enough?
    if (outputBuffer_.size() < toPop)
      outputBuffer_.resize(toPop * 1.1);

    memcpy(&outputBuffer_[0],
	   &buffer_[0] + start_,
	   buffer_.size() - start_);

    memcpy(&outputBuffer_[0] + (buffer_.size() - start_),
	   &buffer_[0],
	   toPop - (buffer_.size() - start_));

    //start_ = toPop - (buffer_.size() - start_);
    //dataSize_ -= toPop;

    return &outputBuffer_[0];
  }
}
