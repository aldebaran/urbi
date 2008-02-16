#ifndef UQUEUE_HXX
# define UQUEUE_HXX

# include "uqueue.hxx"

inline UErrorValue
UQueue::push (const char *s)
{
  return push(reinterpret_cast<const ubyte*>(s), strlen(s));
}

inline int
UQueue::bufferFreeSpace()
{
  return bufferSize_ - dataSize_ ;
}

inline int
UQueue::bufferMaxFreeSpace()
{
  return maxBufferSize_ - dataSize_ ;
}

inline int
UQueue::dataSize()
{
  return dataSize_ ;
}

inline bool
UQueue::locked()
{
  return locked_;
}

inline void
UQueue::setAdaptive (int adaptive)
{
  adaptive_ = adaptive;
}

#endif
