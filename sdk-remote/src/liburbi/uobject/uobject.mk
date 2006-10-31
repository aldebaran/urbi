# This file assumes that we are in $(srcdir)/uobject, i.e., we are
# included by the directory that has uobject/ (svn:externals or not).

uobject_srcdir = $(srcdir)/uobject

uobject_sources =				\
$(uobject_srcdir)/uobject.h			\
$(uobject_srcdir)/uvalue-common.cc

## ----------- ##
## uobject.h.  ##
## ----------- ##

EXTRA_DIST += $(uobject_srcdir)/template_autogen.pl $(uobject_srcdir)/uobject.template.h
CLEANFILES += $(uobject_srcdir)/uobject.h
BUILT_SOURCES += $(uobject_srcdir)/uobject.h

$(srcdir)/uobject.h: $(uobject_srcdir)/uobject.template.h $(uobject_srcdir)/template_autogen.pl
	rm -f uobject.h.tmp $(uobject_srcdir)/uobject.h
	$(srcdir)/template_autogen.pl $(uobject_srcdir)/uobject.template.h >uobject.h.tmp
# Avoid accidental edition.
	chmod a-w uobject.h.tmp
	mv uobject.h.tmp $(uobject_srcdir)/uobject.h
