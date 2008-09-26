include $(top_srcdir)/flags.mk
AUTOMAKE_OPTIONS += subdir-objects

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

# There is no AM_LDADD currently, which is quite inconsistent...
LDADD = $(AM_LDADD)
AM_LDADD =					\
  $(top_builddir)/src/liburbi/liburbi.la	\
  $(top_builddir)/jpeg-6b/libjpeg.la		\
  $(PTHREAD_LIBS)

bin_PROGRAMS =
include examples/local.mk

SUBDIRS = liburbi .

if EXAMPLES
SUBDIRS += utils
endif

## ------------ ##
## version.hh.  ##
## ------------ ##

REVISION_FILE = version.hh
include $(top_srcdir)/build-aux/revision.mk
