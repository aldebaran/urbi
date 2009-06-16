#ifndef RUNNER_STACKS_HXX
# define RUNNER_STACKS_HXX

namespace runner
{
  inline unsigned
  Stacks::local_pointer() const
  {
    return local_pointer_;
  }

  inline unsigned
  Stacks::captured_pointer() const
  {
    return captured_pointer_;
  }
}

#endif
