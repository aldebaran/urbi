include $(top_srcdir)/flags.mk

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

AM_LDADD = $(top_builddir)/src/liburbi/liburbi.la 	\
           $(top_builddir)/jpeg-6b/libjpeg.la		\
           $(PTHREAD_LIBS)

# This is a hack for Automake 1.9: 1.10 honors AM_LDADD.
LDADD = $(AM_LDADD)

bin_PROGRAMS =					\
urbiping

urbiping_SOURCES = urbiping.cc
