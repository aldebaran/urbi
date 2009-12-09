# Yes, this is not by the book, but it is so cooler.
dist_urbi_DATA := $(call ls_files,share/urbi/*.u)
dist_urbi_DATA += share/urbi/tutorial-content.u

nodist_urbi_DATA =				\
  share/urbi/platform.u
BUILT_SOURCES += $(nodist_urbi_DATA)


## ------------ ##
## PackageInfos ##
## ------------ ##

REVISION = $(build_aux_dir)/git-version-gen
REVISIONFLAGS = --urbi --directory
REVISION_RUN = $(REVISION) $(REVISIONFLAGS) --cache=$< --output=$@

nodist_urbi_DATA +=				\
  share/urbi/package-info-urbi-sdk.u		\
  share/urbi/package-info-urbi-sdk-remote.u	\
  share/urbi/package-info-libport.u

share/urbi/package-info-urbi-sdk.u: $(top_srcdir)/.version $(REVISION)
	$(REVISION_RUN) --prefix='Urbi SDK'
	touch $@
share/urbi/package-info-urbi-sdk-remote.u: $(top_srcdir)/sdk-remote/.version $(REVISION)
	$(REVISION_RUN) --prefix='Urbi SDK Remote'
	touch $@
share/urbi/package-info-libport.u: $(top_srcdir)/sdk-remote/libport/.version $(REVISION)
	$(REVISION_RUN) --prefix='Libport'
	touch $@


## ---------- ##
## Tutorial.  ##
## ---------- ##

tutorial_sources =				\
  share/urbi/tutorial/tutorial.xml		\
  share/urbi/tutorial/tutorial.py
EXTRA_DIST += $(tutorial_sources)
$(srcdir)/share/urbi/tutorial-content.u: $(tutorial_sources)
	rm -f $@ $@.tmp
	$(srcdir)/share/urbi/tutorial/tutorial.py $< > $@.tmp
	mv $@.tmp $@

urbi.stamp: $(dist_urbi_DATA) $(nodist_urbi_DATA)
	@echo "$$(date)"': $?' >>$@
