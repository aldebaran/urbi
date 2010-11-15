## Copyright (C) 2007-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

# Define devdir to empty if this file is loaded from dev/Makefile.am,
# or to "dev/" if this file is loaded from the top-level.

EXTRA_DIST +=					\
  $(devdir)ast-all-gen				\
  $(devdir)ast-cloner-gen			\
  $(devdir)ast-transformer-gen			\
  $(devdir)ast-default-visitor-gen		\
  $(devdir)ast-dot-printer-gen			\
  $(devdir)ast-fwd-gen				\
  $(devdir)ast-gen.mk				\
  $(devdir)ast-graph-gen			\
  $(devdir)ast-ignores-gen			\
  $(devdir)ast-nodes-gen			\
  $(devdir)ast-nodes-mk-gen			\
  $(devdir)ast-pretty-printer-gen		\
  $(devdir)ast-readme-gen			\
  $(devdir)ast-visitor-gen			\
  $(devdir)ast.py				\
  $(devdir)tools.py				\
  $(devdir)transform-all-gen			\
  $(devdir)transform-proxy-visitors-gen
