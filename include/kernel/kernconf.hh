#ifndef KERNEL_KERNCONF_HH
#define KERNEL_KERNCONF_HH

# include <cstring>
# include <urbi/export.hh>

struct URBI_SDK_API kernconf_type {
  /// Default and minimum stack size for jobs (in bytes)
  size_t default_stack_size;
  size_t minimum_stack_size;
};

extern URBI_SDK_API kernconf_type kernconf;

#endif // KERNEL_KERNCONF_HH
