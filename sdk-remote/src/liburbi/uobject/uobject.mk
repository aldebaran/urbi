# This file assumes that we are in $(srcdir)/uobject, i.e., we are
# included by the directory that has uobject/ (svn:externals or not).

uobject_srcdir = $(srcdir)/uobject
uobject_hh = $(uobject_srcdir)/uobject.hh

uobject_sources =				\
$(uobject_srcdir)/uobject.h			\
$(uobject_srcdir)/uvalue-common.cc

## ----------- ##
## uobject.h.  ##
## ----------- ##

EXTRA_DIST +=					\
$(uobject_srcdir)/template_autogen.pl		\
$(uobject_srcdir)/uobject.template.h
MAINTAINERCLEANFILES += $(uobject_hh)
BUILT_SOURCES += $(uobject_hh)

$(uobject_hh): $(uobject_hh).template $(uobject_srcdir)/template_autogen.pl
	rm -f uobject.hh.tmp $(uobject_hh)
	$(uobject_srcdir)/template_autogen.pl $(uobject_hh).template >uobject.hh.tmp
# Avoid accidental edition.
	chmod a-w uobject.hh.tmp
	mv uobject.hh.tmp $(uobject_hh)
