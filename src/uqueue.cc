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
#include <libport/cstring>

// #define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include "parser/utoken.hh"

#include "uqueue.hh"

enum
{
  MIN_BUFFER_SIZE = 4096,
  MAX_BUFFER_SIZE = 1048576,
};

UQueue::UQueue ()
  : minBufferSize_ (MIN_BUFFER_SIZE),
    maxBufferSize_ (MAX_BUFFER_SIZE),
    buffer_(minBufferSize_),
    outputBuffer_(UQueue::INITIAL_BUFFER_SIZE),
    start_(0),
    end_(0),
    dataSize_(0)
{
}

UQueue::~UQueue()
{
}

void
UQueue::clear()
{
  start_    = 0;
  end_      = 0;
  dataSize_ = 0;
}

void
UQueue::enlarge (size_t& s) const
{
  s *= 2;
  if (maxBufferSize_)
    s = std::min(s, maxBufferSize_);
}


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
      enlarge(newSize);

      // Realloc the internal buffer
      size_t old_size = buffer_.size();
      buffer_.resize(newSize);
      // If the current chunk is split at the end of the buffer,
      // glue it here.
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

  // Do we have to split 'buffer'?
  if (buffer_.size() - end_ >= length)
  {
    // No need to split.
    memcpy(&buffer_[0] + end_, buffer, length);
    end_ += length;
    if (end_ == buffer_.size())
      // loop the circular geometry.
      end_ = 0;
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

  if (buffer_.size() - start_ >= toPop)
  {
    // Is the packet continuous across the the internal buffer?  yes,
    // the packet is continuous in the internal buffer
    size_t start = start_;
    start_ += toPop;
    if (start_ == buffer_.size())
      start_ = 0; // loop the circular geometry.
    dataSize_ -= toPop;
    return &buffer_[0] + start;
  }
  else
  {
    // no, the packet spans then end and the beginning of the buffer
    // and it must be reconstructed.

    // Is the temporary internal outputBuffer large enough?
    if (outputBuffer_.size() < toPop)
      outputBuffer_.resize(toPop * 2);

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

ubyte*
UQueue::fastPop (size_t &length)
{
  return pop((length > buffer_.size() - start_)
	     ? (length = buffer_.size() - start_)
	     : length);
}


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
      outputBuffer_.resize(toPop * 2);

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

std::string
UQueue::pop_command ()
{
  char *buf = reinterpret_cast<char*>(virtualPop(dataSize()));
  size_t len = prescan(buf);
  buf = reinterpret_cast<char*>(pop(len));
  assert(buf);
  std::string res (buf, len);
  return res;
}
