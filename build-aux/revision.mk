## Build $(REVISION_FILE) from $(REVISION_FILE).in replacing @ID@, @DATE@,
## @REV@ from the svn keywords of the ChangeLog.
##
## Requires a definition of REVISION_FILE.
##
## We use a stamp to avoid updating the revision file too often, for
## instance when the ChangeLog is modified but not checked in.  This
## way, we avoid useless compilation.

BUILT_SOURCES += $(REVISION_FILE)
REVISION_FILE_IN = $(srcdir)/$(REVISION_FILE).in
REVISION_FILE_STAMP = $(REVISION_FILE).stamp

EXTRA_DIST += $(REVISION_FILE_IN)
CLEANFILES =  $(REVISION_FILE) $(REVISION_FILE_STAMP)

$(REVISION_FILE_STAMP): $(top_srcdir)/ChangeLog $(REVISION_FILE_IN) $(top_srcdir)/build-aux/revision.mk
	@rm -f $(REVISION_FILE_STAMP).tmp
	@touch $(REVISION_FILE_STAMP).tmp
# Be sure not to have `/' in Id.  The embedded date may be
# separated by `/' instead of `-', what sed dislikes.
	Date=`sed -n '/^\$$Date: \(.*\) \$$$$/{s,,\1,;s,/,-,g;p;q;}' $(top_srcdir)/ChangeLog`; \
	Id=`sed -n '/^\$$Id: \(.*\) \$$$$/{s,,\1,;s,/,-,g;p;q;}' $(top_srcdir)/ChangeLog`; \
	Rev=`sed -n '/^\$$Rev: \(.*\) \$$$$/{s,,\1,;s,/,-,g;p;q;}' $(top_srcdir)/ChangeLog`; \
	sed -e "s/@DATE@/$$Date/;s/@ID@/$$Id/;s/@REV@/$$Rev/;" $(REVISION_FILE_IN) >$(REVISION_FILE).tmp
	$(top_srcdir)/build-aux/move-if-change $(REVISION_FILE).tmp $(REVISION_FILE)
	@mv -f $(REVISION_FILE_STAMP).tmp $(REVISION_FILE_STAMP)

$(REVISION_FILE): $(REVISION_FILE_STAMP)
	@if test -f $(REVISION_FILE); then :; else \
	  rm -f $(REVISION_FILE_STAMP); \
	  $(MAKE) $(AM_MAKEFLAGS) $(REVISION_FILE_STAMP); \
	fi
