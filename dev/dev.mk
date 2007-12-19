# Define devdir to empty is this file is load from dev/Makefile.am,
# or to "dev/" if this file is loaded from the top-level.

EXTRA_DIST +=					\
$(devdir)ast-all-gen				\
$(devdir)ast-cloner-gen				\
$(devdir)ast-default-visitor-gen		\
$(devdir)ast-fwd-gen				\
$(devdir)ast-gen.mk				\
$(devdir)ast-graph-gen				\
$(devdir)ast-ignores-gen			\
$(devdir)ast-nodes-gen				\
$(devdir)ast-nodes-mk-gen			\
$(devdir)ast-pretty-printer-gen			\
$(devdir)ast-readme-gen				\
$(devdir)ast-visitor-gen			\
$(devdir)ast.py					\
$(devdir)tools.py				\
$(devdir)transform-all-gen			\
$(devdir)transform-proxy-visitors-gen
