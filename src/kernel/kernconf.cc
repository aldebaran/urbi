#include <libport/config.h>
#include <kernel/kernconf.hh>

URBI_SDK_API kernconf_type kernconf = {
  // Default is large enough until we enable a mechanism to dynamically
  // reallocate task space on demand.
  /* .default_stack_size = */         LIBPORT_URBI_KERNEL_STACK_SIZE * 1024,
  /* .minimum_stack_size = */         LIBPORT_URBI_KERNEL_STACK_SIZE * 1024 / 8
};
