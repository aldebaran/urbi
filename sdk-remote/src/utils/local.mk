## Copyright (C) 2008-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

bin_PROGRAMS +=					\
  utils/urbi-cycle				\
  utils/urbi-reverse				\
  utils/urbi-scale

utils_urbi_cycle_SOURCES =			\
  utils/parse-header.hh				\
  utils/urbi-cycle.cc
utils_urbi_reverse_SOURCES =			\
  utils/parse-header.hh				\
  utils/urbi-reverse.cc
utils_urbi_scale_SOURCES =			\
  utils/parse-header.hh				\
  utils/urbi-scale.cc
