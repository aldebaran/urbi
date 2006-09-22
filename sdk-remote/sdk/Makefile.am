TARGETNAME = remote

all: mkgccversion copyfiles

mkgccversion:
	$(CXX) -v 2>&1 |grep version > gccversion

gccversion: mkgccversion

AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)
AM_CPPFLAGS =                                   \
-I$(top_srcdir)/src/liburbi/uobject             \
-I$(top_srcdir)/src/liburbi

#removed, use standard method
#EXTRALDFLAGS = $(LIB_DIR:%=-L%)
#EXTRALDFLAGS += $(LIBS:%=-l%)

## What is this??? -- Akim
copyfiles:
	rm -f liburbi.la uobject.h libuobject.la
	cp $(top_builddir)/src/liburbi/liburbi.la .
	cp $(top_srcdir)/src/liburbi/uobject/uobject.h .
	cp $(top_builddir)/src/liburbi/uobject/libuobject.la .

noinst_PROGRAMS = urbimodule
urbimodule_SOURCES = umain.cc
urbimodule_LDADD = \
	$(top_builddir)/src/liburbi/uobject/libuobject.la \
	$(top_builddir)/src/liburbi/liburbi.la \
	$(PTHREAD_LIBS) \
	$(EXTRALDFLAGS)
urbimodule_LDFLAGS = -L$(prefix)/share/urbicore/remote

umain.cc:
	cp $(prefix)/share/urbicore/remote/umain.cc .

## FIXME: These install instructions are really weird.  Why don't
## we put things where they ought to be? -- Akim.  And copying .a
## or .la does not make sense.
install-data-local:
	mkdir -p  $(prefix)/share/urbicore/remote
	cp Makefile.param umain.cc gccversion libuobject.la liburbi.la  $(prefix)/share/urbicore/remote
	cp uobject.h  $(prefix)/share/urbicore/include
	sed -e "s@PREFIX=.*@PREFIX=$(prefix)@" Makefile.module > $(prefix)/share/urbicore/Makefile.module
	sed -e "s@prefix=.*@prefix=$(prefix)@" Makefile > $(prefix)/share/urbicore/remote/Makefile

sanity:
	$(CXX) -v 2>&1 |grep version | cmp $(prefix)/share/urbicore/remote/gccversion || echo "WARNING: gcc version mismatch"

CLEANFILES = gccversion
