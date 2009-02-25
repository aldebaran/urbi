## -------------------------- ##
## Installed kernel headers.  ##
## -------------------------- ##

kernelincludekerneldir = $(kernelincludedir)/kernel
if INSTALL_KERNEL_HEADERS
dist_kernelincludekernel_HEADERS =		\
  include/kernel/fwd.hh				\
  include/kernel/uconnection.hh			\
  include/kernel/uconnection.hxx		\
  include/kernel/userver.hh			\
  include/kernel/userver.hxx			\
  include/kernel/utypes.hh
endif


## ------- ##
## Check.  ##
## ------- ##

.PHONY: check-headers
check-local: check-headers
check-headers:
# It is forbidden for public headers to depend on non public headers.
# But for 2.0b1, we don't care...
	-srcdir=$(srcdir) \
	  $(srcdir)/include/check-headers $(dist_kernelincludekernel_HEADERS)

EXTRA_DIST +=					\
  include/check-headers
