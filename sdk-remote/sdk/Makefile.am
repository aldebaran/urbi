include $(top_srcdir)/flags.mk

nodist_bin_SCRIPTS = umake
dist_bin_SCRIPTS = umake-lib umake-engine uinstall

host_SCRIPTS = $(top_builddir)/libtool

noinst_PROGRAMS = urbimodule
urbimodule_SOURCES = umain.cc
urbimodule_LDADD = \
	$(top_builddir)/src/liburbi/liburbi.la \
	$(PTHREAD_LIBS)

include_HEADERS = $(top_srcdir)/src/liburbi/uobject/uobject.hh
nodist_host_DATA = param.mk gccversion
#dist_env_DATA = umain.cc

## ----------------- ##
## Makefile.module.  ##
## ----------------- ##

EXTRA_DIST += module.mk.in
module.mk: module.mk.in
	sed -e "s@PREFIX=.*@PREFIX=$(prefix)@" $(srcdir)/module.mk.in >$@.tmp
	mv $@.tmp $@
CLEANFILES += module.mk
