AM_CPPFLAGS =                                   \
-I$(top_srcdir)/src/liburbi/uobject             \
-I$(top_srcdir)/src/liburbi
AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)

noinst_PROGRAMS = urbimodule
urbimodule_SOURCES = umain.cc
urbimodule_LDADD = \
	$(top_builddir)/src/liburbi/uobject/libuobject.la \
	$(top_builddir)/src/liburbi/liburbi.la \
	$(PTHREAD_LIBS)

include_HEADERS = $(top_srcdir)/src/liburbi/uobject/uobject.h
nodist_env_DATA = param.mk module.mk Makefile gccversion
dist_env_DATA = umain.cc

## ----------------- ##
## Makefile.module.  ##
## ----------------- ##

EXTRA_DIST = module.mk.in
module.mk: module.mk.in
	sed -e "s@PREFIX=.*@PREFIX=$(prefix)@" $(srcdir)/module.mk.in >$@.tmp
	mv $@.tmp $@
CLEANFILES = module.mk


install-data-local:
	sed -e "s@prefix=.*@prefix=$(prefix)@" Makefile \
	   >$(DESTDIR)$(envdir)/Makefile

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
