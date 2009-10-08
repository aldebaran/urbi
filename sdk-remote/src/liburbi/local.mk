## ------------ ##
## liburbi.la.  ##
## ------------ ##

lib_LTLIBRARIES += liburbi/liburbi.la
# FIXME: Something is fishy here: why do we duplicate libuco code in
# both liburbi and libuobject?  Libuobject includes liburbi.
liburbi_liburbi_la_SOURCES =			\
  liburbi/compatibility.hh			\
  liburbi/compatibility.hxx			\
  liburbi/kernel-version.cc			\
  liburbi/uabstractclient.cc			\
  liburbi/urbi-launch.cc                        \
  liburbi/uclient.cc				\
  liburbi/uconversion.cc			\
  liburbi/umessage.cc				\
  liburbi/usyncclient.cc			\
  liburbi/utag.cc

liburbi_liburbi_la_CPPFLAGS =			\
  $(AM_CPPFLAGS)				\
  -DBUILDING_URBI_SDK
# Find version.hh.
liburbi_liburbi_la_CPPFLAGS += -I.

liburbi_liburbi_la_LIBADD = $(LIBADD) libuvalue/libuvalue.la $(JPEG_LIBS)
liburbi_liburbi_la_LDFLAGS = -avoid-version -no-undefined
# We are using Libport.OptionParser which uses Boost.Optional, which is
# incompatible with strict alias analysis.
liburbi_liburbi_la_CXXFLAGS = -fno-strict-aliasing $(AM_CXXFLAGS)

## ---------- ##
## umain.cc.  ##
## ---------- ##

dist_umain_DATA = liburbi/umain.cc
