## Copyright (C) 2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

urbi_uobjects_LTLIBRARIES =

# urbi/logger.
urbi_uobjects_LTLIBRARIES += object/urbi/logger.la
dist_object_urbi_logger_la_SOURCES = object/urbi/logger.cc object/urbi/logger.hh
object_urbi_logger_la_LDFLAGS = -module -avoid-version

# urbi/process.
# Currently too hard to make portable.
if !WIN32
urbi_uobjects_LTLIBRARIES += object/urbi/process.la
dist_object_urbi_process_la_SOURCES =		\
  object/urbi/process.cc			\
  object/urbi/process.hh
object_urbi_process_la_LDFLAGS = -module -avoid-version
endif !WIN32

# urbi/regexp.
urbi_uobjects_LTLIBRARIES += object/urbi/regexp.la
dist_object_urbi_regexp_la_SOURCES = object/urbi/regexp.cc object/urbi/regexp.hh
object_urbi_regexp_la_LDFLAGS = -module -avoid-version

# urbi/stream.
urbi_uobjects_LTLIBRARIES += object/urbi/stream.la
dist_object_urbi_stream_la_SOURCES =		\
  object/urbi/stream.cc				\
  object/urbi/stream.hh				\
  object/urbi/input-stream.cc			\
  object/urbi/input-stream.hh			\
  object/urbi/output-stream.cc			\
  object/urbi/output-stream.hh
object_urbi_stream_la_LDFLAGS = -module -avoid-version

