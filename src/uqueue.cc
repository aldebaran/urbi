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

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include "parser/utoken.hh"

#include "uqueue.hh"

UQueue::UQueue ()
  : buffer_(),
    outputBuffer_()
{
}

UQueue::~UQueue()
{
}

void
UQueue::clear()
{
  buffer_.clear();
  outputBuffer_.clear();
}

void
UQueue::push (const ubyte *buf, size_t len)
{
  buffer_.insert(buffer_.end(), buf, buf + len);
}

ubyte*
UQueue::front (size_t len)
{
  // FIXME: Our interface is bad, this is needed.
  if (!len)
  {
    static ubyte res = 0;
    return &res;
  }
  // Not enough data to pop 'len'.
  else if (buffer_.size() < len)
    return 0;

  outputBuffer_.clear();
  outputBuffer_.insert(outputBuffer_.begin(),
		       buffer_.begin(), buffer_.begin() + len);
  outputBuffer_.push_back(0);
  ECHO("Output: "
       << std::string(reinterpret_cast<char*>(&outputBuffer_[0]),
		      len));
  ECHO("Output size: " << outputBuffer_.size());
  return &outputBuffer_[0];
}

ubyte*
UQueue::pop (size_t len)
{
  ubyte* res = front(len);
  buffer_.erase(buffer_.begin(), buffer_.begin() + len);
  return res;
}

std::string
UQueue::pop_command ()
{
  ECHO("Size: " << buffer_.size());
  char *buf = reinterpret_cast<char*>(front(buffer_.size()));
  ECHO("buf: {{{" << std::string(buf) << "}}}");
  size_t len = prescan(buf);
  ECHO("Len: " << len);
  buf = reinterpret_cast<char*>(pop(len));
  assert(buf);
  std::string res (buf, len);
  ECHO("Res: " << res);
  return res;
}
