include $(top_srcdir)/build-aux/build-aux.mk

# That's where aclocal extensions are.
ACLOCAL_AMFLAGS = -I build-aux -I sdk

SUBDIRS =					\
	jpeg-6b libport libport/tests		\
	sdk src .				\
	doc
SVN_EXTERNALS += jpeg-6b sdk libport


## ---------- ##
## include/.  ##
## ---------- ##

# Backward compatibility.
include_HEADERS =				\
include/uobject.h

urbiinclude_HEADERS =				\
include/urbi/uabstractclient.hh			\
include/urbi/uclient.hh				\
include/urbi/uconversion.hh			\
include/urbi/uexternal.hh			\
include/urbi/usyncclient.hh

# liburbi is installed in $prefix, and its headers depend on libport.
# So we must provide libport in $prefix, although they are also
# installed as part of the SDK.  Using symlinks is not portable.
include_libport_dir = $(includedir)/libport
nodist_include_libport__HEADERS = $(libport_HEADERS) $(nodist_libport_HEADERS)
include_libport_sys_dir = $(include_libport_dir)/sys
nodist_include_libport_sys__HEADERS = $(libportsys_HEADERS)

# Neutralize what we had done before with symlinks before running the
# new style installation.  To be removed eventually.
install-data-local:
	test -d $(include_libport_dir) || rm -rf $(include_libport_dir)
