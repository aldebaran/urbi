#include "kernel/kernconf.hh"

kernconf_type kernconf = {
  // Default is large enough until we enable a mechanism to dynamically
  // reallocate task space on demand.
  /* .default_stack_size = */         128 * 1024,
  /* .minimum_stack_size = */         4 * 1024
};
