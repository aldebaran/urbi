# Yes, this is not by the book, but it is so cooler.
dist_urbi_DATA := $(call ls_files,share/urbi/*.u)

nodist_urbi_DATA =				\
  share/urbi/package-info.u			\
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

urbi.stamp: $(dist_urbi_DATA) $(nodist_urbi_DATA)
	@echo "$$(date)"': $?' >>$@
