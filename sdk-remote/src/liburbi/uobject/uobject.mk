# This file assumes that we are in $(srcdir)/uobject, i.e., we are
# included by the directory that has uobject/ (svn:externals or not).

uobject_srcdir = $(srcdir)/uobject
uobject_hh = $(uobject_srcdir)/urbi/uobject.hh

uobject_sources =				\
$(uobject_hh)					\
$(uobject_srcdir)/uvalue-common.cc

EXTRA_DIST += $(uobject_srcdir)/uvalue-common.cc

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

