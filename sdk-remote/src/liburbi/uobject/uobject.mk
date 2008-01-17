# This file assumes that we are in $(srcdir)/uobject, i.e., we are
# included by the directory that has uobject/ (svn:externals or not).

## --------------------- ##
## Sources and headers.  ##
## --------------------- ##

#uobject_srcdir = $(srcdir)/uobject
ucallbacks_hh = $(uobject_srcdir)/urbi/ucallbacks.hh

nodist_uobject_headers =			\
$(ucallbacks_hh)

dist_uobject_headers =				\
$(uobject_srcdir)/urbi/fwd.hh			\
$(uobject_srcdir)/urbi/ubinary.hh		\
$(uobject_srcdir)/urbi/ublend-type.hh		\
$(uobject_srcdir)/urbi/uobject.hh		\
$(uobject_srcdir)/urbi/uproperty.hh		\
$(uobject_srcdir)/urbi/ustarter.hh		\
$(uobject_srcdir)/urbi/usystem.hh		\
$(uobject_srcdir)/urbi/uvalue.hh		\
$(uobject_srcdir)/urbi/uvar.hh

dist_uobject_sources =				\
$(uobject_srcdir)/uobject-hub-common.cc		\
$(uobject_srcdir)/uobject-common.cc		\
$(uobject_srcdir)/uvalue-common.cc		\
$(uobject_srcdir)/uvar-common.cc

uobject_headers = $(nodist_uobject_headers) $(dist_uobject_headers)
uobject_sources = $(nodist_uobject_sources) $(dist_uobject_sources)

EXTRA_DIST += $(dist_uobject_headers) $(dist_uobject_sources)

## --------------- ##
## ucallbacks.hh.  ##
## --------------- ##

EXTRA_DIST +=					\
$(uobject_srcdir)/template_autogen.pl		\
$(ucallbacks_hh).template
MAINTAINERCLEANFILES += $(ucallbacks_hh)
BUILT_SOURCES += $(ucallbacks_hh)

$(ucallbacks_hh): $(ucallbacks_hh).template $(uobject_srcdir)/template_autogen.pl
	rm -f $@.tmp $@
	$(uobject_srcdir)/template_autogen.pl $(ucallbacks_hh).template >$@.tmp
# Avoid accidental edition.
	chmod a-w $@.tmp
	mv $@.tmp $@
