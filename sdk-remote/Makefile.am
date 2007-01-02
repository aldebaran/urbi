include $(top_srcdir)/build-aux/build-aux.mk
include $(top_srcdir)/libport/libport.mk

# That's where aclocal extensions are.
ACLOCAL_AMFLAGS = -I build-aux -I sdk

SUBDIRS = jpeg-6b libport/tests src doc sdk
SVN_EXTERNALS += jpeg-6b sdk


## ---------- ##
## include/.  ##
## ---------- ##

# Backward compatibility.
include_HEADERS =				\
include/uobject.h				\

urbiinclude_HEADERS =				\
include/urbi/uabstractclient.hh			\
include/urbi/uclient.hh				\
include/urbi/uconversion.hh			\
include/urbi/uexternal.hh			\
include/urbi/usyncclient.hh
