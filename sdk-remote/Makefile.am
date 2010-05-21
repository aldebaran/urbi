include build-aux/init.mk
include build-aux/build-aux.mk
include build-aux/package-version.mk
include build-aux/fix-libtool-la.mk
include build-aux/build-farm.mk
BOOTSTRAP_CFG = .bootstrap.cfg
include build-aux/bootstrap.mk

AUTOMAKE_OPTIONS += subdir-objects
ACLOCAL_AMFLAGS = -I build-aux -I sdk

EXTRA_DIST += LICENSE

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status $@

SUBDIRS =					\
  jpeg						\
  libport					\
  .						\
  src						\
  doc

# Also fix libjpeg.la.
LTLIBRARIES_TO_FIX += jpeg/libjpeg4urbi$(LIBSFX).la

# Work around a problem in Automake that needs to the be tracked.
BUILT_SOURCES +=				\
  $(top_srcdir)/sdk/config.h.in			\
  sdk/config.h


brandlibexec_SCRIPTS =
AM_CPPFLAGS +=					\
  $(LIBPORT_CPPFLAGS)				\
  -I$(srcdir)/lib				\
  $(BOOST_CPPFLAGS)

include sdk/local.mk
include include/local.mk


## ---------------------- ##
## Convenient shortcuts.  ##
## ---------------------- ##

.PHONY: core libs
core libs:
	$(MAKE) $(AM_MAKEFLAGS) -C libport
	$(MAKE) -C src $@


## ------- ##
## Check.  ##
## ------- ##

# check-html, check-clean, check-uob etc.
check-%:
	$(MAKE) $(AM_MAKEFLAGS) -C src/tests $@
installcheck-buildfarm check-buildfarm recheck recheck-html: clean-semaphores
	$(MAKE) $(AM_MAKEFLAGS) -C src $@
	$(MAKE) $(AM_MAKEFLAGS) -C src/tests $@


## ----- ##
## Doc.  ##
## ----- ##

.PHONY: doc html

doc html:
	$(MAKE) $(AM_MAKEFLAGS) -C doc $@
