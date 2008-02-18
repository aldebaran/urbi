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
#include <sstream>
#include <strstream>

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include "parser/uparser.hh"

#include "uqueue.hh"

UQueue::UQueue (size_t minBufferSize, size_t maxBufferSize, size_t adaptive)
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
UQueue::mark()
{
  mark_ = end_;
  locked_ = false;
}

void
UQueue::revert()
{
  end_ = mark_;
  locked_ = true;
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

void
UQueue::adapt(size_t toPop)
{
  ++nbPopCall_;
  topDataSize_ = std::max (topDataSize_, dataSize_);
  topOutputSize_ = std::max (topOutputSize_, toPop);

  if (adaptive_ < nbPopCall_)
  {
    // time out
    if (topOutputSize_ < outputBuffer_.size() * 0.8)
      outputBuffer_.resize(topOutputSize_ * 2);

    if (topDataSize_ < buffer_.size() * 0.8)
    {
      // We shrink the buffer to the new size: topDataSize_ + 10% (if it fits)
      enlarge(topDataSize_);
      if (end_ < start_)
      {
	// The data is splitted
	//  [4567|       }      |123]
	//      end     new   start
	// ->
	//  [4567|   |123]
	memmove(&buffer_[0] + start_ - (buffer_.size() - topDataSize_),
		&buffer_[0] + start_, buffer_.size() - start_);
	start_ = start_ - (buffer_.size() - topDataSize_);
      }
      else
      {
	// The data is contiguous.
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

    // reset.
    nbPopCall_ = 0;
    topDataSize_   = 0;
    topOutputSize_ = 0;
  }
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

  // Adaptive shrinking behavior
  if (adaptive_)
    adapt(toPop);

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
  yyFlexLexer scanner;
  // It has been said Flex scanners cannot work with istrstream.
  // Get the whole contents of the UQueue.  This is suboptimal, but
  // functional.  FIXME: spare memory.
  std::istrstream mem_buff (reinterpret_cast<char*>(virtualPop(dataSize())));
  std::istream mem_input (mem_buff.rdbuf());
  scanner.switch_streams(&mem_input, 0);

  // A stack of expected closing braces: ), ], } etc.
  std::vector<char> closers;

  // The result.
  std::string res;

  // The number of byte that were read.
  size_t length = 0;
  while (int c =
	 scanner.yylex(reinterpret_cast<yy::parser::semantic_type*>(&length)))
  {
    ECHO(length << ": " << c << " (" << char(c) << "), closers: "
	 << closers.size());
    switch (c)
    {
      case '{':
	closers.push_back('}');
	break;
      case '[':
	closers.push_back(']');
	break;
      case '(':
	closers.push_back(')');
	break;
      case ')':
      case ']':
      case '}':
	if (!closers.empty() && closers.back() == c)
	  closers.pop_back();
	else
	  // This is a syntax error.  Empty the set of closers so
	  // that we finish as if the sentence was correct.  It will
	  // be given to the parser which will report the error
	  // itself.
	  closers.clear();
	break;

      case ',':
      case ';':
	if (closers.empty())
	{
	  res = std::string(reinterpret_cast<char*>(pop(length)), length);
	  goto done;
	}
	break;

      default:
	pabort (c);
    }
  }
  // We can't break a loop in a switch with break...
  done:
  ECHO("res: {{{" << res << "}}}");
  return res;
}
