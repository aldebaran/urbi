ast_basedir = ast/
ast_srcdir = $(srcdir)/ast
# Don't use ast_srcdir in the include, otherwise Automake can't resolve it.
include $(srcdir)/ast/ast-nodes.mk

EXTRA_DIST += ast/ast.yml
BUILT_SOURCES += $(BUILT_SOURCES_ast)

# Don't use LOCAL_BUILT_SOURCES since it looks like the SOURCES of the
# program `LOCAL_BUILT' and triggers Automake warnings.
#
# List only *generated* files here.
BUILT_SOURCES_ast_libuobject =			\
  ast/fwd.hh					\
  ast/all.hh					\
  ast/dot-printer.cc                            \
  ast/dot-printer.hh                            \
  ast/ignores					\
  ast/visitor.hh				\
  ast/cloner.hh					\
  ast/cloner.cc					\
  ast/default-visitor.hh			\
  ast/default-visitor.hxx			\
  ast/pretty-printer.hh				\
  ast/pretty-printer.cc				\
  ast/transformer.cc				\
  ast/transformer.hh				\
  $(AST_NODES)


BUILT_SOURCES_ast = 				\
  $(BUILT_SOURCES_ast_libuobject) 		\
  ast/dot-printer.hh				\
  ast/dot-printer.cc

gen_dir = $(top_srcdir)/dev
ast_gen_deps =					\
  $(gen_dir)/ast.py				\
  $(gen_dir)/tools.py $(ast_srcdir)/ast.yml

include $(top_srcdir)/dev/ast-gen.mk

dist_libuobject_la_SOURCES +=			\
  ast/all.hh					\
  ast/analyzer.hh				\
  ast/analyzer.cc				\
  ast/catches-type.hh				\
  ast/catches-type.cc				\
  ast/cloner.hxx				\
  ast/dot-print.cc                              \
  ast/dot-print.hh                              \
  ast/error.hh					\
  ast/error.hxx					\
  ast/error.cc					\
  ast/exps-type.hh				\
  ast/exps-type.cc				\
  ast/flavor.cc					\
  ast/flavor.hh					\
  ast/flavor.hxx				\
  ast/loc.hh					\
  ast/local-declarations-type.cc		\
  ast/local-declarations-type.hh		\
  ast/nary-fwd.hh				\
  ast/new-clone.hh				\
  ast/new-clone.hxx				\
  ast/parametric-ast.hh				\
  ast/parametric-ast.hxx			\
  ast/parametric-ast.cc				\
  ast/print.hh ast/print.cc			\
  ast/symbols-type.hh				\
  ast/symbols-type.cc				\
  ast/visitor.hxx

if ENABLE_SERIALIZATION
  dist_libuobject_la_SOURCES +=                 \
    ast/serialize.cc                            \
    ast/serialize.hh
endif

# These files are generated at bootstrap time, we need to ship them.
# We would like to generate them at make time, but unfortunately we
# also generate the Makefile snippets, which is something we cannot do
# easily after Automake was run.
dist_libuobject_la_SOURCES +=			\
  $(BUILT_SOURCES_ast_libuobject)

# To ensure that the file is kept up to date.
dist_noinst_DATA += ast/ast.dot
