AM_CPPFLAGS =                                   \
-I$(top_srcdir)					\
-I$(top_srcdir)/src/liburbi/uobject             \
-I$(top_srcdir)/src/liburbi
AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)

nodist_bin_SCRIPTS = umake
dist_bin_SCRIPTS = umake-lib umake-engine uinstall

host_SCRIPTS = $(top_builddir)/libtool

noinst_PROGRAMS = urbimodule
urbimodule_SOURCES = umain.cc
urbimodule_LDADD = \
	$(top_builddir)/src/liburbi/liburbi.la \
	$(PTHREAD_LIBS)

include_HEADERS = $(top_srcdir)/src/liburbi/uobject/uobject.h
nodist_host_DATA = param.mk gccversion
#dist_env_DATA = umain.cc

## ----------------- ##
## Makefile.module.  ##
## ----------------- ##

EXTRA_DIST = module.mk.in
module.mk: module.mk.in
	sed -e "s@PREFIX=.*@PREFIX=$(prefix)@" $(srcdir)/module.mk.in >$@.tmp
	mv $@.tmp $@
CLEANFILES = module.mk



## ------------ ##
## gccversion.  ##
## ------------ ##

CLEANFILES += gccversion
gccversion:
	$(CXX) -v 2>&1 | grep version >$@.tmp || true
	mv $@.tmp $@

sanity: gccversion
	cmp $(DESTDIR)$(envdir)/gccversion gccversion || \
	  echo >&2 "WARNING: gcc version mismatch"
