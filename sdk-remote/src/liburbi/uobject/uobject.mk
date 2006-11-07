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

svn_externals = $(top_srcdir)/build-aux/svn-externals
# We use some of its files, so to ensure a correct synchronization, we
# pin the revision we're using.  These targets simplify upgrading the
# pin revision.
.PHONY: uobj-up uobject-up uobj-ci uobject-ci uobj-help

help: uobj-help
uobj-help:
	@echo "uobj-up:    same as baux-up, but for uobj"
	@echo "uobj-ci:    likewise"

# Update the dependency on uobj.
uobj-up uobject-up:
	$(svn_externals) --update=uobject $(srcdir)

# Checkin the sub uobj, and update the dependency.
uobj-ci uobject-ci:
	cd $(srcdir)/uobject && $(SVN) ci
	$(MAKE) $(AM_MAKEFLAGS) uobj-up
