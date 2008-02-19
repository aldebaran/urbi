include $(top_srcdir)/build-aux/init.mk

# Find libport's sources and config.h.
AM_CPPFLAGS += $(LIBPORT_CPPFLAGS)
# Find libjpeg's source and build (jconfig.h) headers.
AM_CPPFLAGS += -I$(top_srcdir) -I$(top_builddir)/jpeg-6b
# Find urbi/ headers.
AM_CPPFLAGS += -I$(top_srcdir)/include
# Find urbi/uobject.hh.
AM_CPPFLAGS += -I$(top_srcdir)/src/liburbi/uobject
# Find auto-generated urbi/ucallback.hh.
AM_CPPFLAGS += -I$(top_builddir)/src/liburbi/uobject
# Find version.hh.
AM_CPPFLAGS += -I$(top_builddir)/src
# Find sdk/config.h
AM_CPPFLAGS += -I$(top_builddir)

AM_CXXFLAGS += $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)
LIBADD       = $(top_builddir)/jpeg-6b/libjpeg.la $(PTHREAD_LIBS)
LIBADD      += $(top_builddir)/lib/libport/libport.la
