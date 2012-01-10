## Copyright (C) 2007-2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

## ----------------- ##
## Regular sources.  ##
## ----------------- ##

dist_libuobject@LIBSFX@_la_SOURCES +=		\
  object/barrier.cc				\
  object/centralized-slots.cc			\
  object/code.hh			        \
  object/code.cc				\
  object/cxx-helper.hh				\
  object/cxx-object.cc				\
  object/date.cc				\
  object/dictionary.cc				\
  object/directory.cc				\
  object/duration.cc				\
  object/event.cc				\
  object/event-handler.cc			\
  object/executable.cc				\
  object/file.cc				\
  object/finalizable.cc				\
  object/finalizable.hh				\
  object/float.cc				\
  object/global.cc				\
  object/hash-slots.hh				\
  object/hash-slots.hxx				\
  object/hash.cc				\
  object/ioservice.cc				\
  object/ioservice.hh				\
  object/job.cc					\
  object/list.cc				\
  object/lobby.cc				\
  object/location.cc				\
  object/object-class.cc			\
  object/object-class.hh			\
  object/object.cc				\
  object/object-size.cc				\
  object/path.cc				\
  object/position.cc				\
  object/primitive.cc				\
  object/profile.cc				\
  object/profile.hh				\
  object/register.cc				\
  object/root-classes.cc			\
  object/root-classes.hh			\
  object/semaphore.cc				\
  object/semaphore.hh				\
  object/server.cc                              \
  object/server.hh                              \
  object/slot.cc				\
  object/socket.cc                              \
  object/socket.hh                              \
  object/sorted-vector-slots.hh			\
  object/sorted-vector-slots.hxx		\
  object/string.cc				\
  object/symbols.cc				\
  object/system.cc				\
  object/system.hh				\
  object/tag.cc					\
  object/uconnection.cc				\
  object/uconnection.hh				\
  object/urbi-exception.cc			\
  object/uvalue.cc				\
  object/uvalue.hh				\
  object/uvar.cc				\
  object/uvar.hh				\
  object/vector-slots.hh			\
  object/vector-slots.hxx

if !COMPILATION_MODE_SPACE
  # urbi/format.
  dist_libuobject@LIBSFX@_la_SOURCES +=		\
    object/format-info.cc			\
    object/format-info.hh			\
    object/format-info.hxx			\
    object/formatter.cc				\
    object/formatter.hh
endif !COMPILATION_MODE_SPACE

include object/urbi/local.mk
