# This file assumes that we are in $(srcdir)/uobject, i.e., we are
# included by the directory that has uobject/ (svn:externals or not).

## --------------------- ##
## Sources and headers.  ##
## --------------------- ##

uobject_srcdir = $(srcdir)/uobject
uobject_hh = $(uobject_srcdir)/urbi/uobject.hh

nodist_uobject_headers =			\
$(uobject_hh)

dist_uobject_headers =				\
$(uobject_srcdir)/urbi/utypes-common.hh         \
$(uobject_srcdir)/urbi/usystem.hh

dist_uobject_sources =				\
$(uobject_srcdir)/uvalue-common.cc		\
$(uobject_srcdir)/uvar-common.cc

uobject_headers = $(nodist_uobject_headers) $(dist_uobject_headers)
uobject_sources = $(nodist_uobject_sources) $(dist_uobject_sources)

EXTRA_DIST += $(dist_uobject_headers) $(dist_uobject_sources)

## ------------ ##
## uobject.hh.  ##
## ------------ ##

EXTRA_DIST +=					\
$(uobject_srcdir)/template_autogen.pl		\
$(uobject_hh).template
MAINTAINERCLEANFILES += $(uobject_hh)
BUILT_SOURCES += $(uobject_hh)

$(uobject_hh): $(uobject_hh).template $(uobject_srcdir)/template_autogen.pl
	rm -f uobject.hh.tmp $(uobject_hh)
	$(uobject_srcdir)/template_autogen.pl $(uobject_hh).template >uobject.hh.tmp
# Avoid accidental edition.
	chmod a-w uobject.hh.tmp
	mv uobject.hh.tmp $(uobject_hh)


## ------------ ##
## Svn sugars.  ##
## ------------ ##

# svn-externals.mk must be included for this to work.
SVN_EXTERNALS += uobject

