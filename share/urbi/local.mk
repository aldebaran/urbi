## Copyright (C) 2008-2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

urbidir = $(brandsharedir)/urbi
# Beware of the case where we are a tarball: don't have two different
# path to share/urbi/ros components.
dist_urbi_DATA := 				\
  $(filter-out share/urbi/ros/%, 		\
    $(call ls_files,share/urbi/*.u))

nodist_urbi_DATA =				\
  share/urbi/package-info.u			\
  share/urbi/platform$(LIBSFX).u
BUILT_SOURCES += $(nodist_urbi_DATA)

# For some reasons, Automake does not issue a build rule for this one.
share/urbi/platform$(LIBSFX).u: $(top_builddir)/config.status $(top_srcdir)/share/urbi/platform.u.in
	cd $(top_builddir) && $(SHELL) ./config.status $@


urbi.stamp: $(dist_urbi_DATA) $(nodist_urbi_DATA)

## ----------------- ##
## sub-directories.  ##
## ----------------- ##

# package-info/.
urbi_package_infodir = $(urbidir)/package-info
nodist_urbi_package_info_DATA =			\
  share/urbi/package-info/urbi.u		\
  share/urbi/package-info/libport.u

share/urbi/package-info/urbi.u: $(top_srcdir)/.version $(VERSIONIFY)
	$(AM_V_GEN)$(VERSIONIFY_RUN) --urbiscript=$@ --prefix='Urbi'
	$(AM_V_at)touch $@
share/urbi/package-info/libport.u: $(top_srcdir)/sdk-remote/libport/.version $(VERSIONIFY)
	$(AM_V_GEN)$(VERSIONIFY_RUN) --urbiscript=$@ --prefix='Libport'
	$(AM_V_at)touch $@

# ros/.
urbi_rosdir = $(urbidir)/ros
dist_urbi_ros_DATA := $(call ls_files,@{share/urbi/ros}/*)
