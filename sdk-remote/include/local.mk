# Backward compatibility.
include_HEADERS =				\
  include/uobject.h

ucallbacks_hh = include/urbi/ucallbacks.hh

urbiinclude_HEADERS =				\
  $(ucallbacks_hh)				\
  include/urbi/export.hh			\
  include/urbi/fwd.hh				\
  include/urbi/package-info.hh			\
  include/urbi/qt_umain.hh			\
  include/urbi/uabstractclient.hh		\
  include/urbi/ubinary.hh			\
  include/urbi/ublend-type.hh			\
  include/urbi/uclient.hh			\
  include/urbi/uconversion.hh			\
  include/urbi/uexternal.hh			\
  include/urbi/umain.hh				\
  include/urbi/uobject.hh			\
  include/urbi/uproperty.hh			\
  include/urbi/ustarter.hh			\
  include/urbi/ustarter.hxx			\
  include/urbi/usyncclient.hh			\
  include/urbi/usystem.hh			\
  include/urbi/utable.hh			\
  include/urbi/utag.hh				\
  include/urbi/utimer-callback.hh		\
  include/urbi/utimer-table.hh			\
  include/urbi/uvalue.hh			\
  include/urbi/uvar.hh

## --------------- ##
## ucallbacks.hh.  ##
## --------------- ##

EXTRA_DIST +=				\
  include/template_autogen.pl		\
  $(ucallbacks_hh).template
MAINTAINERCLEANFILES += $(ucallbacks_hh)

# We used to generate this file in builddir and not ship it, but it
# required to adjust -I paths to find it in builddir.  Since this was
# the only such file, it is easier to put it in srcdir (and ship it to
# comply with the Automake model).
$(srcdir)/$(ucallbacks_hh): $(ucallbacks_hh).template include/template_autogen.pl
	rm -f $@.tmp $@
	$(mkdir_p) $$(dirname "$@.tmp")
	$(srcdir)/include/template_autogen.pl $(srcdir)/$(ucallbacks_hh).template >$@.tmp
# Avoid accidental edition.
	chmod a-w $@.tmp
	mv $@.tmp $@


# liburbi is installed in $prefix, and its headers depend on libport.
# So we must provide libport in $prefix, although they are also
# installed as part of the SDK.  Using symlinks is not portable.
#include_libport_dir = $(includedir)/libport
#nodist_include_libport__HEADERS = $(libport_HEADERS) $(nodist_libport_HEADERS)
#include_libport_sys_dir = $(include_libport_dir)/sys
#nodist_include_libport_sys__HEADERS = $(libportsys_HEADERS)

include_libport = include/libport
include include/libport/local.mk
