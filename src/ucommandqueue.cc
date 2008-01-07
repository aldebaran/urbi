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
  ADDOBJ(UCommandQueue);
  FREEOBJ(UQueue); // A tester.
}

//! UCommandQueue destructor.
UCommandQueue::~UCommandQueue()
{
  ADDOBJ(UQueue);
  FREEOBJ(UCommandQueue);
}


ubyte*
UCommandQueue::popCommand (int &length)
{
  // Scanning
  for (/* nothing */; cursor_ < dataSize_; ++cursor_)
  {
    // Extract the previous char (' ' otherwise).
    int previousposition = (start_ + cursor_ - 1 + bufferSize_) % bufferSize_;
    char p_1 = cursor_ == 0 ? (char) buffer_[previousposition] : ' ';

    int position = (start_ + cursor_) % bufferSize_;
    char p0 = (char) buffer_[position];

    int nextposition = (position + 1) % bufferSize_;
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

    // , or ; separator, except between paren or brackets
    if (((char)buffer_[position] == ',' || (char)buffer_[position] == ';' )
	&& !discard_
	&& closers_.empty())
    {
      // Include the terminator.
      ++cursor_;
      ubyte* res = pop(cursor_);
      length = cursor_;
      cursor_ = 0;
      ECHO("Sending {{{"
	   << std::string(reinterpret_cast<const char*>(res), length)
	   << "}}}");
      return res;
    }
  }

  // A complete command was not found.
  length = 0;
  return buffer_;
}
