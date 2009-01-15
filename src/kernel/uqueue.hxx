#ifndef KERNEL_UQUEUE_HXX
# define KERNEL_UQUEUE_HXX

namespace kernel
{
  inline size_t
  UQueue::size() const
  {
    return next_character_ - first_character_;
  }

  inline bool UQueue::empty() const
  {
    return !size();
  }

  inline void
  UQueue::push(const char *s)
  {
    push(s, strlen(s));
  }

  inline void
  UQueue::clear()
  {
    first_character_ = next_character_ = buffer_;
  }
}

#endif // !KERNEL_UQUEUE_HXX
