include $(top_srcdir)/flags.mk
AUTOMAKE_OPTIONS += subdir-objects

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

AM_LDADD =					\
  $(top_builddir)/src/liburbi/liburbi.la	\
  $(top_builddir)/jpeg-6b/libjpeg.la		\
  $(PTHREAD_LIBS)

bin_PROGRAMS =
include examples/local.mk
include utils/local.mk

SUBDIRS = liburbi tests .
if BINDINGS
 SUBDIRS += swig
endif

## ------------ ##
## version.hh.  ##
## ------------ ##

REVISION_FILE = version.hh
include $(top_srcdir)/build-aux/revision.mk
