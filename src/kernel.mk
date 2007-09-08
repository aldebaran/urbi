include $(top_srcdir)/build-aux/init.mk
# Get $(libport_la).
include $(top_srcdir)/libport/libport-sources.mk

# Find the kernel public headers.
AM_CPPFLAGS += -I$(top_srcdir)/include

# Find the kernel private headers.
AM_CPPFLAGS += -I$(top_srcdir)/src -I$(top_builddir)/src
# Find the uobject headers
AM_CPPFLAGS += -I$(top_srcdir)/src/uobject

# Find libport headers.
AM_CPPFLAGS += $(LIBPORT_CPPFLAGS)

# Find uobject headers.
AM_CPPFLAGS += -I$(srcdir)/uobject

AM_CXXFLAGS += $(WARNING_CXXFLAGS) $(PTHREAD_CFLAGS)

# Add boost libraries.
AM_CPPFLAGS += $(BOOST_CPPFLAGS)
AM_LDFLAGS  += $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB)
