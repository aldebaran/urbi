urbidir = $(brandsharedir)/urbi
dist_urbi_DATA := $(call ls_files,share/urbi/*.u)

nodist_urbi_DATA =				\
  share/urbi/package-info.u			\
  share/urbi/platform.u
BUILT_SOURCES += $(nodist_urbi_DATA)


## -------------- ##
## PackageInfos.  ##
## -------------- ##

urbi_package_infodir = $(urbidir)/package-info
nodist_urbi_package_info_DATA =			\
  share/urbi/package-info/urbi-sdk.u		\
  share/urbi/package-info/urbi-sdk-remote.u	\
  share/urbi/package-info/libport.u

share/urbi/package-info/urbi-sdk.u: $(top_srcdir)/.version $(VERSIONIFY)
	$(VERSIONIFY_RUN) --urbiscript=$@ --prefix='Urbi SDK'
	touch $@
share/urbi/package-info/urbi-sdk-remote.u: $(top_srcdir)/sdk-remote/.version $(VERSIONIFY)
	$(VERSIONIFY_RUN) --urbiscript=$@ --prefix='Urbi SDK Remote'
	touch $@
share/urbi/package-info/libport.u: $(top_srcdir)/sdk-remote/libport/.version $(VERSIONIFY)
	$(VERSIONIFY_RUN) --urbiscript=$@ --prefix='Libport'
	touch $@

urbi.stamp: $(dist_urbi_DATA) $(nodist_urbi_DATA)
	@echo "$$(date)"': $?' >>$@
