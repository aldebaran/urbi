/// \file uqueue.cc
/// \brief Implementation of UQueue.

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include <libport/assert.hh>
#include <libport/cstring>

#include <kernel/uqueue.hh>
#include <parser/prescan.hh>

UQueue::UQueue(super_type::size_type chunk_size)
  : super_type(chunk_size)
{
}

std::string
UQueue::pop_command()
{
  // The buffer is null-terminated when used in full, so we can use it
  // without computing its length.
  ECHO("Size: " << size());
  ECHO("buf: {{{" << std::string(peek()) << "}}}");
  size_t len = parser::prescan(peek());
  ECHO("Len: " << len);
  const std::string res(pop(len), len);
  ECHO("Res: " << res);
  return res;
}
