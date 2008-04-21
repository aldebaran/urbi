#ifndef KERNEL_KERNCONF_HH
#define KERNEL_KERNCONF_HH

# include <cstring>

struct kernconf_type {
  /// Default and minimum stack size for jobs (in bytes)
  size_t default_stack_size;
  size_t minimum_stack_size;
};

extern kernconf_type kernconf;

#endif // KERNEL_KERNCONF_HH
