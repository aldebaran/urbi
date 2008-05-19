/// \file uqueue.cc
/// \brief Implementation of UQueue.

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include <cstdlib>
#include <libport/cstring>

#include <cassert>
#include <string>

#include "parser/prescan.hh"

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
UQueue::push (const char *buf, size_t len)
{
  buffer_.insert(buffer_.end(), buf, buf + len);
}

char*
UQueue::front (size_t len)
{
  // FIXME: Our interface is bad, this is needed.
  if (!len)
  {
    static char res = 0;
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
       << std::string(&outputBuffer_[0], len));
  ECHO("Output size: " << outputBuffer_.size());
  return &outputBuffer_[0];
}

char*
UQueue::pop (size_t len)
{
  char* res = front(len);
  buffer_.erase(buffer_.begin(), buffer_.begin() + len);
  return res;
}

std::string
UQueue::pop_command ()
{
  ECHO("Size: " << buffer_.size());
  char *buf = front(buffer_.size());
  ECHO("buf: {{{" << std::string(buf) << "}}}");
  size_t len = parser::prescan(buf);
  ECHO("Len: " << len);
  buf = pop(len);
  assert(buf);
  std::string res (buf, len);
  ECHO("Res: " << res);
  return res;
}
