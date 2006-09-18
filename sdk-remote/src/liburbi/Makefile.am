if OPENR
SUBDIRS = openr .
else
SUBDIRS = uobject .
endif 


AM_CPPFLAGS = \
-I$(top_srcdir)/lib \
-I$(top_builddir)/src/liburbi/uobject \
-I$(top_srcdir)/src/liburbi/uobject

AM_LDFLAGS = -L$(top_srcdir)/lib/jpeg-6b
AM_CXXFLAGS = $(PTHREAD_LIBS) $(WARNING_CXXFLAGS)


## ----------------------- ##
## Doxygen documentation.  ##
## ----------------------- ##

all: $(DOCDIR)
$(DOCDIR): Doxyfile.in
	rm -rf $(DOCDIR).tmp $(DOCDIR)
	$(MAKE) $(AM_MAKEFLAGS) Doxyfile
	$(DOXYGEN) Doxyfile
	mv $(DOCDIR).tmp $(DOCDIR)

clean-local:
	rm -rf $(DOCDIR).tmp $(DOCDIR)


## ------------ ##
## liburbi.la.  ##
## ------------ ##

lib_LTLIBRARIES = liburbi.la

include_HEADERS =				\
lockable.h					\
uabstractclient.h				\
uclient.h					\
uconversion.h					\
uobject/uexternal.h				\
uobject/uobject.h				\
usyncclient.h

BUILT_SOURCES = uobject/uobject.h
uobject/uobject.h: uobject/uobject.template.h
	cd uobject && $(MAKE) uobject.h

liburbi_la_SOURCES =				\
uabstractclient.h	uabstractclient.cpp	\
uclient.h		uclient.cpp		\
uconversion.h		uconversion.cpp		\
uobject/common_uvalue.cc			\
uobject/uexternal.h				\
uobject/uobject.h	uobject/uobject.cc	\
uobject/uvar.cc					\
usyncclient.h		usyncclient.cpp

liburbi_la_LIBADD = $(top_builddir)/lib/jpeg-6b/libjpeg.la
