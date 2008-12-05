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
#
# We depend on .version to avoid frequent regeneration of this file.
#
# Alternatively you could make it "PHONY" in which case Make knows it
# has to recreate each time "make" is run.  Of course in that case you
# really want to use "move-if-change" to avoid updating the timestamp
# of this file, so that we don't have to recompile all its
# dependencies.
#
# But that does not work because in that case, as it is PHONY, when
# "recreated" the file is recent in the tables of Make (although its
# actual timestamp was not changed on the disk), so it wants to
# recompile all the files that depend on it.  In other words,
# "move-if-change" is useless :(
#
# Why don't we have the same problem with ".version"? (Indeed, it is
# declared PHONY, so recreated all the time, but then since
# git-version.hh depends on it, it will be recreated etc.).  Because
# it is not in the same Makefile (Makefile vs. src/Makefile).
BUILT_SOURCES += kernel/git-version.hh
kernel/git-version.hh: $(top_srcdir)/.version
	$(build_aux_dir)/git-version-gen	\
		--srcdir=$(top_srcdir)		\
		--header --output=$@

CLEANFILES += kernel/git-version.hh
