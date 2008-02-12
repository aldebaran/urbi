include $(top_srcdir)/build-aux/init.mk
# Get $(libport_la).
# lib_libport = ../lib/libport
# include $(top_srcdir)/lib/libport/libport-sources.mk

# Find the kernel private headers.  location.hh etc. are generated.
AM_CPPFLAGS += -I$(srcdir) -I.

# Find the uobject headers
AM_CPPFLAGS += -I$(srcdir)/uobject -Iuobject

# Find the kernel, libport, etc. headers.
AM_CPPFLAGS += -I$(top_srcdir)/include -I$(top_builddir)/include

# lib/network
AM_CPPFLAGS += -I$(top_srcdir)/lib

# Find sdk/config.h.
AM_CPPFLAGS += -I$(top_builddir)

# Find uobject headers.
AM_CPPFLAGS += -I$(srcdir)/uobject

AM_CXXFLAGS += $(WARNING_CXXFLAGS) $(PTHREAD_CFLAGS)

# Add boost libraries.
AM_CPPFLAGS += $(BOOST_CPPFLAGS)
AM_LDFLAGS  += $(BOOST_THREAD_LDFLAGS)
# There is no AM_LIBADD.
LIBS        += $(BOOST_THREAD_LIBS)
