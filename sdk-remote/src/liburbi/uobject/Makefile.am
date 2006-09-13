AM_CPPFLAGS = -I$(top_srcdir)/src/liburbi
AM_CXXFLAGS = $(PTHREAD_LIBS) $(WARNING_CXXFLAGS)
lib_LTLIBRARIES = libuobject.la

libuobject_la_SOURCES =				\
common_uvalue.cc				\
uexternal.h 					\
uobject.h		uobject.cc		\
uvar.cc

EXTRA_DIST = template_autogen.pl uobject.template.h
CLEANFILES = uobject.h
BUILT_SOURCES = uobject.h
uobject.h: $(srcdir)/uobject.template.h $(srcdir)/template_autogen.pl
	rm -f uobject.h.tmp uobject.h
	$(srcdir)/template_autogen.pl $(srcdir)/uobject.template.h >uobject.h.tmp
# Avoid accidental edition.
	chmod a-w uobject.h.tmp
	mv uobject.h.tmp uobject.h
