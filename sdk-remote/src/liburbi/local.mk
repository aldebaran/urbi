AM_CPPFLAGS += $(BOOST_CPPFLAGS)

umaindir = $(datadir)/umain
dist_umain_DATA = liburbi/umain.cc

## ------------ ##
## liburbi.la.  ##
## ------------ ##

lib_LTLIBRARIES += liburbi/liburbi.la
liburbi_liburbi_la_SOURCES =			\
  libuco/uvalue-common.cc			\
  liburbi/package-info.cc			\
  liburbi/uabstractclient.cc			\
  liburbi/uclient.cc				\
  liburbi/uconversion.cc			\
  liburbi/umessage.cc				\
  liburbi/usyncclient.cc
liburbi_liburbi_la_CPPFLAGS =			\
  $(AM_CPPFLAGS)				\
  -DBUILDING_URBI_SDK
liburbi_liburbi_la_LIBADD = $(LIBADD) $(top_builddir)/jpeg-6b/libjpeg.la
liburbi_liburbi_la_LDFLAGS = -avoid-version -no-undefined

all-local: liburbi/liburbi.la.stamp

liburbi/liburbi.la.stamp: liburbi/liburbi.la
	$(build_aux_dir)/fix-libtool-la liburbi/liburbi.la \
	  liburbi/liburbi.la.stamp

CLEANFILES += liburbi/liburbi.la.stamp
