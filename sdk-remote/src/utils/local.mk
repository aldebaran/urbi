if EXAMPLES
bin_PROGRAMS +=					\
  utils/urbicycle				\
  utils/urbireverse				\
  utils/urbiscale				\
  utils/urbiwalk
endif

utils_urbicycle_SOURCES =			\
  utils/parse-header.hh				\
  utils/urbicycle.cc
utils_urbireverse_SOURCES =			\
  utils/parse-header.hh				\
  utils/urbireverse.cc
utils_urbiscale_SOURCES =			\
  utils/parse-header.hh				\
  utils/urbiscale.cc

utils_urbiwalk_SOURCES =			\
  utils/urbiwalk.cc				\
  utils/move.cc					\
  utils/move.hh

utils_urbiwalk_LDADD =				\
  $(top_builddir)/jpeg-6b/libjpeg.la		\
  liburbi/liburbi.la
