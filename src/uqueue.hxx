#ifndef UQUEUE_HXX
# define UQUEUE_HXX

# include "uqueue.hxx"

inline
bool
UQueue::empty () const
{
  return !dataSize_;
}

inline
UErrorValue
UQueue::push (const char *s)
{
  return push(reinterpret_cast<const ubyte*>(s), strlen(s));
}

inline
size_t
UQueue::bufferFreeSpace()
{
  return buffer_.size() - dataSize_ ;
}

inline
size_t
UQueue::bufferMaxFreeSpace()
{
  return maxBufferSize_ - dataSize_ ;
}

inline
size_t
UQueue::dataSize()
{
  return dataSize_ ;
}

inline
bool
UQueue::locked()
{
  return locked_;
}

#endif
