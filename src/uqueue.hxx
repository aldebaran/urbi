#ifndef UQUEUE_HXX
# define UQUEUE_HXX

# include "uqueue.hxx"

inline
size_t
UQueue::size () const
{
  return buffer_.size();
}

inline
bool
UQueue::empty () const
{
  return buffer_.empty();
}

inline
void
UQueue::push (const char *s)
{
  push(s, strlen(s));
}

#endif
