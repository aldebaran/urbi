include $(top_srcdir)/build-aux/build-aux.mk

# That's where aclocal extensions are.
ACLOCAL_AMFLAGS = -I build-aux

SUBDIRS = jpeg-6b libport src doc sdk

# libport.
include $(top_srcdir)/libport/libport.mk


## ---------- ##
## include/.  ##
## ---------- ##

urbiinclude_HEADERS =				\
include/urbi/uabstractclient.hh			\
include/urbi/uclient.hh				\
include/urbi/uconversion.hh			\
include/urbi/uexternal.hh			\
include/urbi/usyncclient.hh
