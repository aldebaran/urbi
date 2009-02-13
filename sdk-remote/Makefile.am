include build-aux/init.mk
include build-aux/build-aux.mk
include build-aux/package-version.mk
include build-aux/fix-libtool-la.mk
AUTOMAKE_OPTIONS += subdir-objects
ACLOCAL_AMFLAGS = -I build-aux -I sdk

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status $@

SUBDIRS =					\
  jpeg						\
  libport					\
  .						\
  src						\
  doc

host_SCRIPTS =
nodist_host_DATA =
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
	$(MAKE) $(AM_MAKEFLAGS) -C lib/libport
	$(MAKE) -C src $@


## ------- ##
## Check.  ##
## ------- ##

# check-html, check-clean, check-uob etc.
check-%:
	$(MAKE) $(AM_MAKEFLAGS) -C src/tests $@
check-buildfarm recheck recheck-html:
	$(MAKE) $(AM_MAKEFLAGS) -C src/tests $@


## ----- ##
## Doc.  ##
## ----- ##

.PHONY: doc html

doc html:
	$(MAKE) $(AM_MAKEFLAGS) -C doc $@
