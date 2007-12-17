#ifndef KERNEL_UWAIT_COUNTER_HH
# define KERNEL_UWAIT_COUNTER_HH

/// The number of pending call to a remote new for a given class name (id).
class UWaitCounter
{
public:
  UWaitCounter(const UString& id, int nb)
    : id(id),
      nb(nb)
  {
  }

  UString id; ///< class name
  int nb; ///< nb of waiting calls
};

#endif
