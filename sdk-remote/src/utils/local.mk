bin_PROGRAMS +=					\
  utils/urbi-cycle				\
  utils/urbi-reverse				\
  utils/urbi-scale				\
  utils/urbi-walk

utils_urbi_cycle_SOURCES =			\
  utils/parse-header.hh				\
  utils/urbi-cycle.cc
utils_urbi_reverse_SOURCES =			\
  utils/parse-header.hh				\
  utils/urbi-reverse.cc
utils_urbi_scale_SOURCES =			\
  utils/parse-header.hh				\
  utils/urbi-scale.cc

utils_urbi_walk_SOURCES =			\
  utils/urbi-walk.cc				\
  utils/move.cc					\
  utils/move.hh
utils_urbi_walk_LDADD =				\
  $(JPEG_LIBS)					\
  liburbi/liburbi.la
