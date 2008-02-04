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
    cursor_ (0),
    close_ (0),
    closers_ ()
{
}

//! UCommandQueue destructor.
UCommandQueue::~UCommandQueue()
{
}


std::string
UCommandQueue::popCommand ()
{
 // Scanning
  for (/* nothing */; cursor_ < dataSize_; ++cursor_)
  {
    int position = (start_ + cursor_) % bufferSize_;
    char p0 = (char) buffer_[position];

    int nextposition = (position + 1) % bufferSize_;
    char p1 = (cursor_ < dataSize_ - 1) ? (char) buffer_[nextposition] : 0;

    // Beware of \ which disables the next char whatever the context.
    if (p0 == '\\')
    {
      ++cursor_;
      continue;
    }

    // In string or in comment.
    if (close_)
    {
      // Look for the closing sequence.
      if (p0 == close_[0])
      {
	if (// One-char close sequence: strings, // and # comments.
	  close_[1] == 0
	  // Two-char close sequences: /* ... */
	  || p1 == close_[1])
	  close_ = 0;
      }
    }
    else
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
	  close_ = "\n";
	  break;

	case '"':
	  close_ = "\"";
	  break;

	// FIXME: It is totally broken with 'd, 'n, etc.
	// There is no easy way to handle this without rewritting a lexer.
	// case '\'':
	  // close_ = "'";
	  // break;

	case '/':
	  if (p1 == '*')
	  {
	    close_ = "*/";
	    ++cursor_;
	  }
	  else if (p1 == '/')
	  {
	    close_ = "\n";
	    ++cursor_;
	  }
	  break;

	case ',':
	case ';':
	  // , or ; separator, except between paren or brackets
	  if (closers_.empty())
	  {
	    // Include the terminator.
	    ++cursor_;
	    std::string res;
	    res = std::string (reinterpret_cast<const char*> (pop(cursor_)),
			cursor_);
	    cursor_ = 0;  
	    ECHO("Sending {{{" << res << "}}}");
	    return res;
	  }
      }
  }

  // A complete command was not found.
  return std::string();
}
