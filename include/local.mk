## -------------------------- ##
## Installed kernel headers.  ##
## -------------------------- ##

if INSTALL_KERNEL_HEADERS
kernelinclude_kerneldir = $(kernelincludedir)/kernel
dist_kernelinclude_kernel_HEADERS =		\
  include/kernel/fwd.hh				\
  include/kernel/uconnection.hh			\
  include/kernel/uconnection.hxx		\
  include/kernel/userver.hh			\
  include/kernel/userver.hxx			\
  include/kernel/utypes.hh

kernelinclude_urbidir = $(kernelincludedir)/urbi
dist_kernelinclude_urbi_HEADERS =		\
  include/urbi/sdk.hh				\
  include/urbi/sdk.hxx

kernelinclude_urbi_objectdir = $(kernelincludedir)/urbi/object
dist_kernelinclude_urbi_object_HEADERS =	\
  include/urbi/object/any-to-boost-function.hh	\
  include/urbi/object/any-to-boost-function.hxx	\
  include/urbi/object/barrier.hh		\
  include/urbi/object/centralized-slots.hh	\
  include/urbi/object/centralized-slots.hxx	\
  include/urbi/object/code.hh			\
  include/urbi/object/cxx-conversions.hh	\
  include/urbi/object/cxx-conversions.hxx	\
  include/urbi/object/cxx-object.hh		\
  include/urbi/object/cxx-object.hxx		\
  include/urbi/object/cxx-primitive.hh		\
  include/urbi/object/cxx-primitive.hxx		\
  include/urbi/object/date.hh			\
  include/urbi/object/dictionary.hh		\
  include/urbi/object/directory.hh		\
  include/urbi/object/equality-comparable.hh	\
  include/urbi/object/equality-comparable.hxx	\
  include/urbi/object/executable.hh		\
  include/urbi/object/file.hh			\
  include/urbi/object/float.hh			\
  include/urbi/object/fwd.hh			\
  include/urbi/object/global.hh			\
  include/urbi/object/list.hh			\
  include/urbi/object/lobby.hh			\
  include/urbi/object/object.hh			\
  include/urbi/object/object.hxx		\
  include/urbi/object/path.hh			\
  include/urbi/object/primitive.hh		\
  include/urbi/object/slot.hh			\
  include/urbi/object/slot.hxx			\
  include/urbi/object/string.hh			\
  include/urbi/object/tag.hh			\
  include/urbi/object/task.hh

kernelinclude_urbi_runnerdir = $(kernelincludedir)/urbi/runner
dist_kernelinclude_urbi_runner_HEADERS =	\
  include/urbi/runner/raise.hh
endif


## --------- ##
## install.  ##
## --------- ##

install-data-hook: install-boost-headers

install-boost-headers:
	sh -x $(builddir)/scripts/install-boost-headers
