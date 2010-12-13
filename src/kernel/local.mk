## Copyright (C) 2008-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

dist_libuobject@LIBSFX@_la_SOURCES +=		\
  kernel/connection.cc				\
  kernel/connection.hh				\
  kernel/connection-set.cc			\
  kernel/connection-set.hh			\
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
  kernel/userver.cc				\
  kernel/usystem.cc				\
  kernel/uvalue-cast.cc				\
  kernel/uvalue-cast.hh

# revision-stub.hh.
#
# We do not depend upon .version, because we do not want to be
# refreshed too frequently.  That's the whole point of a stub.
kernel/revision-stub.hh: $(VERSIONIFY)
	$(VERSIONIFY_RUN) --stub=$@
nodist_libuobject@LIBSFX@_la_SOURCES += kernel/revision-stub.hh

# Resolve stubs in libubanner, because at install time, we relink urbi
# and so forth to use this library.  As a consequence, we'd have old
# versions of the revision info.
all-local: kernel/ubanner.unstub.stamp
kernel/ubanner.unstub.stamp: kernel/libuobject$(LIBSFX)_la-ubanner.lo $(VERSIONIFY) $(VERSIONIFY_CACHE)
	$(VERSIONIFY_RUN) --resolve=$<
