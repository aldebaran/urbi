## --------------------- ##
## Sources and headers.  ##
## --------------------- ##

ucallbacks_hh = libuco/urbi/ucallbacks.hh

noinst_LTLIBRARIES = libuco/libuco.la

nodist_urbiinclude_HEADERS =			\
  $(ucallbacks_hh)

dist_urbiinclude_HEADERS =			\
  libuco/urbi/export.hh				\
  libuco/urbi/fwd.hh				\
  libuco/urbi/qt_umain.hh			\
  libuco/urbi/ubinary.hh			\
  libuco/urbi/ublend-type.hh			\
  libuco/urbi/umain.hh				\
  libuco/urbi/uobject.hh			\
  libuco/urbi/uproperty.hh			\
  libuco/urbi/ustarter.hh			\
  libuco/urbi/ustarter.hxx			\
  libuco/urbi/usystem.hh			\
  libuco/urbi/utable.hh				\
  libuco/urbi/utimer-callback.hh		\
  libuco/urbi/utimer-table.hh			\
  libuco/urbi/uvalue.hh				\
  libuco/urbi/uvar.hh

dist_libuco_libuco_la_SOURCES =			\
  libuco/uobject-hub-common.cc			\
  libuco/utable.cc				\
  libuco/utimer-table.cc			\
  libuco/uvalue-common.cc			\
  libuco/urbi-main.cc				\
  libuco/uvar-common.cc

## --------------- ##
## ucallbacks.hh.  ##
## --------------- ##

EXTRA_DIST +=					\
  libuco/template_autogen.pl		\
  $(ucallbacks_hh).template
MAINTAINERCLEANFILES += $(ucallbacks_hh)
BUILT_SOURCES += $(ucallbacks_hh)

$(ucallbacks_hh): $(ucallbacks_hh).template libuco/template_autogen.pl
	rm -f $@.tmp $@
	$(mkdir_p) $$(dirname "$@.tmp")
	$(srcdir)/libuco/template_autogen.pl $(srcdir)/$(ucallbacks_hh).template >$@.tmp
# Avoid accidental edition.
	chmod a-w $@.tmp
	mv $@.tmp $@
