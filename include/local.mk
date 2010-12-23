## Copyright (C) 2008-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

## -------------------------- ##
## Installed kernel headers.  ##
## -------------------------- ##

if INSTALL_KERNEL_HEADERS
kernelinclude_kerneldir = $(brandincludedir)/kernel
dist_kernelinclude_kernel_HEADERS =		\
  include/kernel/fwd.hh				\
  include/kernel/uconnection.hh			\
  include/kernel/uconnection.hxx		\
  include/kernel/userver.hh			\
  include/kernel/userver.hxx			\
  include/kernel/utypes.hh

kernelinclude_urbidir = $(brandincludedir)/urbi
dist_kernelinclude_urbi_HEADERS =		\
  include/urbi/sdk.hh				\
  include/urbi/sdk.hxx

kernelinclude_urbi_objectdir = $(brandincludedir)/urbi/object
dist_kernelinclude_urbi_object_HEADERS =	\
  include/urbi/object/any-to-boost-function.hh	\
  include/urbi/object/barrier.hh		\
  include/urbi/object/centralized-slots.hh	\
  include/urbi/object/centralized-slots.hxx	\
  include/urbi/object/code.hh			\
  include/urbi/object/cxx-conversions.hh	\
  include/urbi/object/cxx-conversions.hxx	\
  include/urbi/object/cxx-object.hh		\
  include/urbi/object/cxx-object.hxx		\
  include/urbi/object/cxx-primitive.hh		\
  include/urbi/object/date.hh			\
  include/urbi/object/date.hxx			\
  include/urbi/object/dictionary.hh		\
  include/urbi/object/directory.hh		\
  include/urbi/object/duration.hh		\
  include/urbi/object/duration.hxx		\
  include/urbi/object/enumeration.hh		\
  include/urbi/object/equality-comparable.hh	\
  include/urbi/object/equality-comparable.hxx	\
  include/urbi/object/event.hh			\
  include/urbi/object/event-handler.hh          \
  include/urbi/object/file.hh			\
  include/urbi/object/float.hh			\
  include/urbi/object/float.hxx			\
  include/urbi/object/fwd.hh			\
  include/urbi/object/global.hh			\
  include/urbi/object/hash.hh			\
  include/urbi/object/job.hh			\
  include/urbi/object/list.hh			\
  include/urbi/object/lobby.hh			\
  include/urbi/object/lobby.hxx			\
  include/urbi/object/location.hh		\
  include/urbi/object/location.hxx		\
  include/urbi/object/object.hh			\
  include/urbi/object/object-as.hh		\
  include/urbi/object/object.hxx		\
  include/urbi/object/path.hh			\
  include/urbi/object/position.hh		\
  include/urbi/object/position.hxx		\
  include/urbi/object/primitive.hh		\
  include/urbi/object/slot.hh			\
  include/urbi/object/slot.hxx			\
  include/urbi/object/string.hh			\
  include/urbi/object/tag.hh			\
  include/urbi/object/tag.hxx

kernelinclude_urbi_runnerdir = $(brandincludedir)/urbi/runner
dist_kernelinclude_urbi_runner_HEADERS =	\
  include/urbi/runner/raise.hh
endif INSTALL_KERNEL_HEADERS


## ------------------------ ##
## Generated source files.  ##
## ------------------------ ##

FROM_GEN =					\
  include/urbi/object/any-to-boost-function.hxx	\
  include/urbi/object/cxx-primitive.hxx		\
  include/urbi/object/executable.hh
BUILT_SOURCES += $(FROM_GEN)

if INSTALL_KERNEL_HEADERS
dist_kernelinclude_urbi_object_HEADERS += $(FROM_GEN)
endif INSTALL_KERNEL_HEADERS
EXTRA_DIST += $(FROM_GEN:=.gen)

%: %.gen
	$(AM_V_GEN)mkdir -p $(dir $@)
	$(AM_V_at)$< > $@.tmp
	$(AM_V_at)chmod a-w $@.tmp
	$(AM_V_at)$(top_srcdir)/build-aux/bin/move-if-change --color $@.tmp $@
	$(AM_V_at)touch $@
