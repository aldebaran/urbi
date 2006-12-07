include $(top_srcdir)/build-aux/init.mk

# Find config.h which is in sdk.
AM_CPPFLAGS += $(top_builddir)/sdk
# Find libport's sources and config.h.
AM_CPPFLAGS += $(LIBPORT_CPPFLAGS)
# Find libjpeg's build (jconfig.h) files.
AM_CPPFLAGS += -I$(top_builddir)/jpeg-6b
# Find urbi/ headers.
AM_CPPFLAGS += -I$(top_srcdir)/include
# Find urbi/uobject.hh.
AM_CPPFLAGS += -I$(top_srcdir)/src/liburbi/uobject

AM_CXXFLAGS += $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)
LIBADD       = $(top_builddir)/jpeg-6b/libjpeg.la $(PTHREAD_LIBS)
