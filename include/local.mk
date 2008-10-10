## -------------------------- ##
## Installed kernel headers.  ##
## -------------------------- ##

kernelincludekerneldir = $(kernelincludedir)/kernel
dist_kernelincludekernel_HEADERS =		\
  include/kernel/fwd.hh				\
  include/kernel/kernconf.hh			\
  include/kernel/uconnection.hh			\
  include/kernel/uconnection.hxx		\
  include/kernel/userver.hh			\
  include/kernel/userver.hxx			\
  include/kernel/utypes.hh

nodist_kernelincludekernel_HEADERS =		\
include/kernel/version.hh


## ------- ##
## Check.  ##
## ------- ##

kernel_headers = 				\
  $(dist_kernelincludekernel_HEADERS)		\
  $(nodist_kernelincludekernel_HEADERS)

SKIP_HEADERS_STD = \
  $(EGREP) -v '^<(cstring|string|cstdarg|iomanip|sstream|iosfwd)>$$'

nullstring :=
space := $(nullstring) # end of the line

SKIP_HEADERS_KERNEL_PUBLIC := \
  $(EGREP) -v "^<$(subst $(space),|,$(subst include/,,$(kernel_headers)))>$$"

.PHONY: check-headers
check-local: check-headers
check-headers:
# It is forbidden for public headers to depend on non public headers.
	@echo "Checking whether private headers are included by public headers."
	for i in $(kernelincludekernel_HEADERS);			\
	do								\
	  ! < $(srcdir)/$$i sed -ne '/^# *include */{s///;p;}' |	\
	      $(EGREP) -v '^<(boost|libport)/' |			\
	      $(SKIP_HEADERS_STD) |					\
	      $(SKIP_HEADERS_KERNEL_PUBLIC);				\
	done
