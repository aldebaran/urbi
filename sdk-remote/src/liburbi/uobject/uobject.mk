# This file requires to be included by the Makefile of the parent
# directory.

## --------------------- ##
## Sources and headers.  ##
## --------------------- ##

#uobject_srcdir = $(srcdir)/uobject
ucallbacks_hh = uobject/urbi/ucallbacks.hh

nodist_uobject_headers =			\
$(ucallbacks_hh)

dist_uobject_headers =				\
uobject/urbi/fwd.hh				\
uobject/urbi/ubinary.hh				\
uobject/urbi/ublend-type.hh			\
uobject/urbi/uobject.hh				\
uobject/urbi/uproperty.hh			\
uobject/urbi/ustarter.hh			\
uobject/urbi/ustarter.hxx			\
uobject/urbi/usystem.hh				\
uobject/urbi/utable.hh				\
uobject/urbi/utimer-callback.hh			\
uobject/urbi/utimer-table.hh			\
uobject/urbi/uvalue.hh				\
uobject/urbi/uvar.hh

dist_uobject_sources =				\
uobject/uobject-hub-common.cc			\
uobject/utable.cc				\
uobject/utimer-table.cc				\
uobject/uvalue-common.cc			\
uobject/uvar-common.cc

uobject_headers = $(nodist_uobject_headers) $(dist_uobject_headers)
uobject_sources = $(nodist_uobject_sources) $(dist_uobject_sources)

EXTRA_DIST += $(dist_uobject_headers) $(dist_uobject_sources)

## --------------- ##
## ucallbacks.hh.  ##
## --------------- ##

EXTRA_DIST +=					\
uobject/template_autogen.pl			\
$(ucallbacks_hh).template
MAINTAINERCLEANFILES += $(ucallbacks_hh)
BUILT_SOURCES += $(ucallbacks_hh)

$(ucallbacks_hh): $(ucallbacks_hh).template uobject/template_autogen.pl
	rm -f $@.tmp $@
	$(mkdir_p) $$(dirname "$@.tmp")
	$(srcdir)/uobject/template_autogen.pl $(srcdir)/$(ucallbacks_hh).template >$@.tmp
# Avoid accidental edition.
	chmod a-w $@.tmp
	mv $@.tmp $@
