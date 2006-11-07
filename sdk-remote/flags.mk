include $(top_srcdir)/build-aux/init.mk

# Find libjpeg's src (jpeglib.h) and build (jconfig.h) files.
AM_CPPFLAGS += -I$(top_srcdir) -I$(top_builddir)/jpeg-6b
# Find urbi/ headers.
AM_CPPFLAGS += -I$(top_srcdir)/include
# Find libport's config.h.
AM_CPPFLAGS += -I$(top_builddir)
# Find uobject.hh.
AM_CPPFLAGS += -I$(top_srcdir)/src/liburbi

AM_CXXFLAGS += $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)
LIBADD       = $(top_builddir)/jpeg-6b/libjpeg.la $(PTHREAD_LIBS)
