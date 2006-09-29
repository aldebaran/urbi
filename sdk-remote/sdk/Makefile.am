AM_CPPFLAGS =                                   \
-I$(top_srcdir)/src/liburbi/uobject             \
-I$(top_srcdir)/src/liburbi
AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)

bin_SCRIPTS = umake umake-lib umake-engine

host_SCRIPTS = $(top_builddir)/libtool

noinst_PROGRAMS = urbimodule
urbimodule_SOURCES = umain.cc
urbimodule_LDADD = \
	$(top_builddir)/src/liburbi/uobject/libuobject.la \
	$(top_builddir)/src/liburbi/liburbi.la \
	$(PTHREAD_LIBS)

include_HEADERS = $(top_srcdir)/src/liburbi/uobject/uobject.h
nodist_host_DATA = param.mk gccversion
#dist_env_DATA = umain.cc

install-data-local:
	cp $(top_srcdir)/sdk/install $(prefix)/
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
	$(CXX) -v 2>&1 | grep version >$@.tmp
	mv $@.tmp $@

sanity: gccversion
	cmp $(DESTDIR)$(envdir)/gccversion gccversion || \
	  echo >&2 "WARNING: gcc version mismatch"
