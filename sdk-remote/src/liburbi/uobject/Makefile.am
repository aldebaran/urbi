AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src/liburbi
AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)
# Automake 1.10 uses AM_LDADD, not previous versions.
LDADD = $(PTHREAD_LIBS)

lib_LTLIBRARIES = libuobject.la

libuobject_la_SOURCES =				\
common_uvalue.cc				\
uexternal.h 					\
uobject.h		uobject.cc		\
uvar.cc

# If using ufloats, we need their code.
if UFLOAT
nodist_libuobject_la_SOURCES =			\
$(top_srcdir)/libport/ufloat.cc
endif

## ----------- ##
## uobject.h.  ##
## ----------- ##

EXTRA_DIST = template_autogen.pl uobject.template.h
CLEANFILES = $(srcdir)/uobject.h
BUILT_SOURCES = $(srcdir)/uobject.h
$(srcdir)/uobject.h: $(srcdir)/uobject.template.h $(srcdir)/template_autogen.pl
	rm -f uobject.h.tmp $(srcdir)/uobject.h
	$(srcdir)/template_autogen.pl $(srcdir)/uobject.template.h >uobject.h.tmp
# Avoid accidental edition.
	chmod a-w uobject.h.tmp
	mv uobject.h.tmp $(srcdir)/uobject.h
