## Build $(REVISION_FILE) from $(REVISION_FILE).in replacing @Id@, @Date@,
## @Rev@ from the svn keywords of the ChangeLog.
##
## Requires a definition of REVISION_FILE.

BUILT_SOURCES += $(REVISION_FILE)
EXTRA_DIST += $(REVISION_FILE).in
REVISION_FILE_STAMP = $(REVISION_FILE).stamp
CLEANFILES = $(REVISION_FILE_STAMP)
$(REVISION_FILE_STAMP): $(top_srcdir)/ChangeLog $(srcdir)/$(REVISION_FILE).in
	@rm -f $(REVISION_FILE_STAMP).tmp
	@touch $(REVISION_FILE_STAMP).tmp
# Be sure not to have `/' in Id.  The embedded date may be
# separated by `/' instead of `-', what sed dislikes.
	Date=`sed -n '/^\$$Date/{s,/,-,g;p;q;}' $(top_srcdir)/ChangeLog`; \
	Id=`sed -n '/^\$$Id/{s,/,-,g;p;q;}' $(top_srcdir)/ChangeLog`; \
	Rev=`sed -n '/^\$$Rev/{s,/,-,g;p;q;}' $(top_srcdir)/ChangeLog`; \
	sed -e "s/@DATE@/Date/;s/@Id@/$$Id/;s/@REV@/$$Rev/;" $(srcdir)/$(REVISION_FILE).in >$(REVISION_FILE).tmp
	$(top_srcdir)/build-aux/move-if-change $(REVISION_FILE).tmp $(REVISION_FILE)
	@mv -f $(REVISION_FILE_STAMP).tmp $(REVISION_FILE_STAMP)

$(REVISION_FILE): $(REVISION_FILE_STAMP)
	@if test -f $(REVISION_FILE); then :; else \
	  rm -f $(REVISION_FILE_STAMP); \
	  $(MAKE) $(AM_MAKEFLAGS) $(REVISION_FILE_STAMP); \
	fi
