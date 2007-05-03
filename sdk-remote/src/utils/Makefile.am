include $(top_srcdir)/flags.mk

bin_PROGRAMS = urbicycle urbireverse urbiwalk urbiscale

urbicycle_SOURCES = parse-header.hh urbicycle.cc
urbireverse_SOURCES = parse-header.hh urbireverse.cc
urbiscale_SOURCES = parse-header.hh urbiscale.cc
urbiwalk_SOURCES = urbiwalk.cc move.cc move.hh

urbiwalk_LDADD = $(top_builddir)/jpeg-6b/libjpeg.la $(top_builddir)/src/liburbi/liburbi.la
