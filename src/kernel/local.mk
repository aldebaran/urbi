dist_libuobject_la_SOURCES +=			\
  kernel/connection-set.cc			\
  kernel/connection-set.hh			\
  kernel/debug.cc				\
  kernel/debug.hh				\
  kernel/kernconf.cc				\
  kernel/server-timer.hh			\
  kernel/server-timer.cc			\
  kernel/ubanner.hh				\
  kernel/ubanner.cc				\
  kernel/uconnection.cc				\
  kernel/ughostconnection.hh			\
  kernel/ughostconnection.cc			\
  kernel/utypes.cc				\
  kernel/uobject.cc				\
  kernel/uobject.hh				\
  kernel/uqueue.hh				\
  kernel/uqueue.cc				\
  kernel/userver.cc				\
  kernel/usystem.cc				\
  kernel/uvalue-cast.cc				\
  kernel/uvalue-cast.hh


# Create the file git-config.hh with accurate revision information.
# This is time consumming.  We will try to make it less costly later.
BUILT_SOURCES += kernel/git-version.hh
.PHONY: kernel/git-version.hh
kernel/git-version.hh:
	rm -f $@.tmp
	(cd $(top_srcdir) && build-aux/git-version-gen --header) >$@.tmp
	$(build_aux_dir)/move-if-change $@.tmp $@
