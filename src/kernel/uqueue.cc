/// \file uqueue.cc
/// \brief Implementation of UQueue.

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include <libport/assert.hh>
#include <libport/cstring>

#include <kernel/uqueue.hh>
#include <parser/prescan.hh>

UQueue::UQueue(size_t chunk_size)
  : chunk_size_(chunk_size)
  , buffer_(static_cast<char*>(malloc(chunk_size_)))
  , capacity_(chunk_size_)
  , first_character_(buffer_)
  , next_character_(buffer_)
{
  passert(buffer_, buffer_);
}

UQueue::~UQueue()
{
  free(buffer_);
}

void
UQueue::push(const char *buf, size_t len)
{
  // Compute the new size of the stored data and a null character.
  size_t nsz = size() + len + 1;

  // Check whether we need to move data around, along with a possible
  // larger buffer if needed.
  if (first_character_ + nsz > buffer_ + capacity_)
  {
    // If the existing buffer is already large enough, move data
    // to the beginning, otherwise increase capacity and reallocate.
    if (nsz <= capacity_)
      memmove(buffer_, first_character_, size());
    else
    {
      // Increase the buffer capacity by multiples of chunk_size.
      capacity_ = chunk_size_ * (1 + (nsz - 1) / chunk_size_);

      // Rather than using realloc(), allocate a new buffer so that
      // we do not copy useless data located before first_character_.
      char* old_buffer = buffer_;
      buffer_ = static_cast<char*>(malloc(capacity_));
      passert(buffer_, buffer_);
      memcpy(buffer_, first_character_, size());
      free(old_buffer);
    }

    // Data has been moved, recompute the bounds.
    next_character_ += buffer_ - first_character_;
    first_character_ = buffer_;
  }

  // Append the new data and adjust the next character address.
  memcpy(next_character_, buf, len);
  next_character_ += len;

  // Null-terminate the stored data.
  *next_character_ = '\0';
}

const char*
UQueue::front(size_t len) const
{
  // Return the data if we have enough, 0 otherwise.
  return size() < len ? 0 : first_character_;
}

const char*
UQueue::pop(size_t len)
{
  const char* res = front(len);
  if (res)
  {
    first_character_ += len;

    // If we have just emptied the buffer, start again at the
    // beginning so that no reallocation will be needed in the near
    // future.
    if (empty())
      clear();
  }
  return res;
}

std::string
UQueue::pop_command()
{
  // The buffer is null-terminated when used in full, so we can use it
  // without computing its length.
  ECHO("Size: " << size());
  ECHO("buf: {{{" << std::string(first_character_) << "}}}");
  size_t len = parser::prescan(first_character_);
  ECHO("Len: " << len);
  const std::string res(pop(len), len);
  ECHO("Res: " << res);
  return res;
}
