AM_CPPFLAGS =					\
-I$(top_srcdir)					\
-I$(top_srcdir)/src/liburbi/uobject		\
-I$(top_srcdir)/src/liburbi
AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)

bin_PROGRAMS = urbicycle urbireverse urbiwalk urbiscale

urbicycle_SOURCES = urbicycle.cc
urbireverse_SOURCES = urbireverse.cc
urbiscale_SOURCES = urbiscale.cc
urbiwalk_SOURCES = urbiwalk.cc move.cc move.hh

urbiwalk_LDADD = $(top_builddir)/jpeg-6b/libjpeg.la $(top_builddir)/src/liburbi/liburbi.la
