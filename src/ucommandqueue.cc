/*! \file ucommandqueue.cc
 *******************************************************************************

 File: ucommandqueue.cc\n
 Implementation of the UCommandQueue class.

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

//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"
#include <cstdlib>
#include "libport/cstring"

#include "kernel/userver.hh"

#include "uqueue.hh"
#include "ucommandqueue.hh"


//! UCommandQueue constructor.
/*! UCommandQueue constructor simply calls the UQueue constructor with the same
    behavior.
*/
UCommandQueue::UCommandQueue  (int minBufferSize,
			       int maxBufferSize,
			       int adaptive)
  : UQueue (minBufferSize, maxBufferSize, adaptive),
    cursor_         (0),
    discard_        (false),
    closechar_      (' '),
    closechar2_     (' '),
    closers_()
{
}

//! UCommandQueue destructor.
UCommandQueue::~UCommandQueue()
{
}


ubyte*
UCommandQueue::popCommand (int &length)
{
  if (dataSize_ == 0)
  {
    length = 0;
    return buffer_;
  }

  // Scanning
  int position = start_ + cursor_;
  if (position >= bufferSize_)
    position = position - bufferSize_;

  int nextposition = position + 1;
  if (nextposition >= bufferSize_)
    nextposition = nextposition - bufferSize_;

  int previousposition = position - 1;
  if (previousposition < 0)
    previousposition = previousposition + bufferSize_;
  char p0 = (char) buffer_[previousposition]; // extract the previous char.
  if (cursor_ == 0)
    // no previous char at start
    p0 = ' ';

  bool found = false;
  while (cursor_ < dataSize_ && !found)
  {
    char p_1 = p0;
    p0 = (char) buffer_[position];
    char p1 = (cursor_ < dataSize_ - 1) ? (char) buffer_[nextposition] : '-';

    if (discard_)
    {
      // One char close sequence
      if (p0 == closechar_ && closechar2_ == ' ')
	discard_ = closechar_ == '"' && p_1 == '\\';

      // Two chars close sequence
      if (p_1 == closechar_ && p0  == closechar2_ && closechar2_ != ' ')
	discard_ = false;
    }
    else
    {
      switch (p0)
      {
	case '{':
	  closers_.push_back('}');
	  break;
	case '[':
	  closers_.push_back(']');
	  break;
	case '(':
	  closers_.push_back(')');
	  break;
	case ')':
	case ']':
	case '}':
	  if (!closers_.empty() && closers_.back() == p0)
	    closers_.pop_back();
	  else
	    // This is a syntax error.  Empty the set of closers so
	    // that we finish as if the sentence was correct.  It will
	    // be given to the parser which will report the error
	    // itself.
	    closers_.clear();
	  break;

	case '#':
	  discard_    = true;
	  closechar_  = '\n';
	  closechar2_ = ' ';
	  break;

	case '"':
	  discard_    = true;
	  closechar_  = '"';
	  closechar2_ = ' ';
	  break;

	default:
	  if (p0 == '/' && p1 == '*'
	      || p_1 == '/' && p0 == '*')
	  {
	    discard_    = true;
	    closechar_  = '*';
	    closechar2_ = '/';
	  }
	  if (p0 == '/' && p1 == '/'
	      || p_1 == '/' && p0 == '/')
	  {
	    discard_    = true;
	    closechar_  = '\n';
	    closechar2_ = ' ';
	  }
      }
    }

    // , or ; separator, except between paren or brackets
    if (((char)buffer_[position] == ',' || (char)buffer_[position] == ';' )
	&& !discard_
	&& closers_.empty())
      found = true;

    // Emergency escape character: ¤
    if ((char)buffer_[position] == '$'
	&& (char)buffer_[nextposition] == '$'
	&& !discard_)
    {
      length = -1;
      closers_.clear();
      cursor_ = 0;
      discard_  = false;
      return 0;
    }

    ++cursor_;
    ++position;
    if (position == bufferSize_)
      position = 0;
    ++nextposition;
    if (nextposition == bufferSize_)
      nextposition = 0;
  }

  ubyte* res;
  if (found)
  {
    res = pop(cursor_);
    length = cursor_;
    cursor_ = 0;
  }
  else
  {
    res = buffer_;
    length = 0;
  }
  ECHO("Sending {{{"
       << std::string(reinterpret_cast<const char*>(res), length)
       << "}}}");
  return res;
}
