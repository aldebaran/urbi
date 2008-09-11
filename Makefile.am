include $(top_srcdir)/build-aux/build-aux.mk

ACLOCAL_AMFLAGS = -I build-aux -I sdk

# Process "." before the sub dirs to make sure sdk/config.h and
# libport/config.h are propery built.
SUBDIRS = sdk-remote . src tests doc

# build-aux.
EXTRA_DIST +=					\
build-aux/bison++.in				\
build-aux/move-if-change			\
build-aux/output-to-dot

# dev.
devdir = dev/
include dev/dev.mk

## -------------------------- ##
## Installed kernel headers.  ##
## -------------------------- ##

kernelincludekerneldir = $(kernelincludedir)/kernel
kernelincludekernel_HEADERS =			\
include/kernel/exception.hh			\
include/kernel/exception.hxx			\
include/kernel/fwd.hh				\
include/kernel/kernconf.hh			\
include/kernel/uconnection.hh			\
include/kernel/uconnection.hxx			\
include/kernel/userver.hh			\
include/kernel/userver.hxx			\
include/kernel/utypes.hh

include share/urbi/local.mk

nodist_kernelincludekernel_HEADERS =		\
include/kernel/version.hh



## ------------------------------------- ##
## Generate the list of symbols we use.  ##
## ------------------------------------- ##

.PHONY: symbols
symbols:
	$(MAKE) $(AM_MAKEFLAGS) -C src $@



## ------- ##
## Check.  ##
## ------- ##

kernel_headers = 				\
  $(kernelincludekernel_HEADERS)		\
  $(nodist_kernelincludekernel_HEADERS)

SKIP_HEADERS_STD = \
  $(EGREP) -v '^<(cstring|string|cstdarg|iomanip|sstream|iosfwd)>$$'

nullstring :=
space := $(nullstring) # end of the line

SKIP_HEADERS_KERNEL_PUBLIC := \
  $(EGREP) -v "^<$(subst $(space),|,$(subst include/,,$(kernel_headers)))>$$"

check-local:
# It is forbidden for public headers to depend on non public headers.
	@echo "Checking whether private headers are included by public headers."
	for i in $(kernelincludekernel_HEADERS);			\
	do								\
	  ! < $(srcdir)/$$i sed -ne '/^# *include */{s///;p;}' |	\
	      $(EGREP) -v '^<(boost|libport)/' |			\
	      $(SKIP_HEADERS_STD) |					\
	      $(SKIP_HEADERS_KERNEL_PUBLIC);				\
	done

# check-html, check-clean, check-uob etc.
check-%:
	cd tests && $(MAKE) $(AM_MAKEFLAGS) $@

# The target run by the buildfarm.
CHECK_BUILDFARM_FLAGS = AM_COLOR_TESTS=no VERBOSE=1 INSTRUMENT=1
check-buildfarm:
	cd tests && $(MAKE) $(AM_MAKEFLAGS) check-html $(CHECK_BUILDFARM_FLAGS)

all-local:
	$(MAKE) -C bison MAKEFLAGS=
