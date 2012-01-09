## Copyright (C) 2011-2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

# Common flags.
urbi_uobjects_LDFLAGS = $(AM_LDFLAGS) -module -avoid-version
urbi_uobjects_CPPFLAGS = $(AM_CPPFLAGS) -DBUILDING_URBI_MODULE
# See the comment in src/Makefile.am.
urbi_uobjects_CXXFLAGS = $(AM_CXXFLAGS) -fno-strict-aliasing
if WIN32
urbi_uobjects_LDFLAGS += -no-undefined
urbi_uobjects_LIBADD = libuobject@LIBSFX@.la
endif WIN32

EXTRA_DIST +=					\
  object/urbi/export.hh

urbi_uobjects_LTLIBRARIES =

# urbi/logger.
urbi_uobjects_LTLIBRARIES += object/urbi/logger.la
dist_object_urbi_logger_la_SOURCES = object/urbi/logger.cc object/urbi/logger.hh
object_urbi_logger_la_CPPFLAGS = $(urbi_uobjects_CPPFLAGS)
object_urbi_logger_la_CXXFLAGS = $(urbi_uobjects_CXXFLAGS)
object_urbi_logger_la_LDFLAGS = $(urbi_uobjects_LDFLAGS)
object_urbi_logger_la_LIBADD = $(urbi_uobjects_LIBADD)

# urbi/process.
# Currently too hard to make portable.
if !WIN32
urbi_uobjects_LTLIBRARIES += object/urbi/process.la
dist_object_urbi_process_la_SOURCES =		\
  object/urbi/process.cc			\
  object/urbi/process.hh
object_urbi_process_la_CPPFLAGS = $(urbi_uobjects_CPPFLAGS)
object_urbi_process_la_CXXFLAGS = $(urbi_uobjects_CXXFLAGS)
object_urbi_process_la_LDFLAGS = $(urbi_uobjects_LDFLAGS)
object_urbi_process_la_LIBADD = $(urbi_uobjects_LIBADD)
endif !WIN32

# urbi/regexp.
urbi_uobjects_LTLIBRARIES += object/urbi/regexp.la
dist_object_urbi_regexp_la_SOURCES = object/urbi/regexp.cc object/urbi/regexp.hh
object_urbi_regexp_la_CPPFLAGS = $(urbi_uobjects_CPPFLAGS)
object_urbi_regexp_la_CXXFLAGS = $(urbi_uobjects_CXXFLAGS)
object_urbi_regexp_la_LDFLAGS = $(urbi_uobjects_LDFLAGS) $(BOOST_REGEX_LDFLAGS)
object_urbi_regexp_la_LIBADD = $(urbi_uobjects_LIBADD) $(BOOST_REGEX_LIBS)

# urbi/stream.
urbi_uobjects_LTLIBRARIES += object/urbi/stream.la
dist_object_urbi_stream_la_SOURCES =		\
  object/urbi/stream.cc				\
  object/urbi/stream.hh				\
  object/urbi/input-stream.cc			\
  object/urbi/input-stream.hh			\
  object/urbi/output-stream.cc			\
  object/urbi/output-stream.hh
object_urbi_stream_la_CPPFLAGS = $(urbi_uobjects_CPPFLAGS)
object_urbi_stream_la_CXXFLAGS = $(urbi_uobjects_CXXFLAGS)
object_urbi_stream_la_LDFLAGS = $(urbi_uobjects_LDFLAGS)
object_urbi_stream_la_LIBADD = $(urbi_uobjects_LIBADD)
