## Copyright (C) 2008-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

dist_libuobject@LIBSFX@_la_SOURCES +=			\
  kernel/connection.cc				\
  kernel/connection.hh				\
  kernel/connection-set.cc			\
  kernel/connection-set.hh			\
  kernel/debug.cc				\
  kernel/debug.hh				\
  kernel/lock.cc				\
  kernel/lock.hh				\
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
  kernel/uqueue.hxx				\
  kernel/uqueue.cc				\
  kernel/userver.cc				\
  kernel/usystem.cc				\
  kernel/uvalue-cast.cc				\
  kernel/uvalue-cast.hh


## ------------------------ ##
## kernel/urbi-sdk-key.hh.  ##
## ------------------------ ##

nodist_libuobject@LIBSFX@_la_SOURCES +=			\
  kernel/urbi-sdk-key.hh
dist_noinst_SCRIPTS += kernel/generate-urbi-sdk-key-hh
kernel/urbi-sdk-key.hh: $(top_srcdir)/urbi-sdk.key
	rm -f $@ $@.tmp
	$(srcdir)/kernel/generate-urbi-sdk-key-hh <$< >$@.tmp
	mv $@.tmp $@
