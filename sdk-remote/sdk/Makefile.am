AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)
AM_CPPFLAGS =                                   \
-I$(top_srcdir)/src/liburbi/uobject             \
-I$(top_srcdir)/src/liburbi

#removed, use standard method
#EXTRALDFLAGS = $(LIB_DIR:%=-L%)
#EXTRALDFLAGS += $(LIBS:%=-l%)

noinst_PROGRAMS = urbimodule
urbimodule_SOURCES = umain.cc
urbimodule_LDADD = \
	$(top_builddir)/src/liburbi/uobject/libuobject.la \
	$(top_builddir)/src/liburbi/liburbi.la \
	$(PTHREAD_LIBS) \
	$(EXTRALDFLAGS)
urbimodule_LDFLAGS = -L$(prefix)/share/urbicore/$(TARGET)

## FIXME: These install instructions are really weird.  Why don't
## we put things where they ought to be? -- Akim.  And copying .a
## or .la does not make sense.
include_HEADERS = $(top_srcdir)/src/liburbi/uobject/uobject.h
nodist_target_DATA = Makefile.param Makefile.module Makefile gccversion
dist_target_DATA = umain.cc

## ----------------- ##
## Makefile.module.  ##
## ----------------- ##

EXTRA_DIST = Makefile.module.in
Makefile.module: Makefile.module.in
	sed -e "s@PREFIX=.*@PREFIX=$(prefix)@" $(srcdir)/Makefile.module.in >$@.tmp
	mv $@.tmp $@
CLEANFILES = Makefile.module


install-data-local:
	sed -e "s@prefix=.*@prefix=$(prefix)@" Makefile \
	   >$(DESTDIR)$(targetdir)/Makefile

## ------------ ##
## gccversion.  ##
## ------------ ##

CLEANFILES += gccversion
gccversion:
	$(CXX) -v 2>&1 | grep version >$@.tmp
	mv $@.tmp $@

sanity: gccversion
	cmp $(DESTDIR)$(targetdir)/gccversion gccversion || \
	  echo >&2 "WARNING: gcc version mismatch"
