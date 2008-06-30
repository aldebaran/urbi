ast_basedir = ast/
ast_srcdir = $(srcdir)/ast
# Don't use ast_srcdir in the include, otherwise Automake can't resolve it.
include $(srcdir)/ast/ast-nodes.mk
ast_nodes = $(AST_NODES)

EXTRA_DIST += ast/ast.yml
BUILT_SOURCES += $(BUILT_SOURCES_ast)
# Don't use LOCAL_BUILT_SOURCES since it looks like the SOURCES of the
# program `LOCAL_BUILT' and triggers Automake warnings.
BUILT_SOURCES_ast =                             \
ast/fwd.hh                                      \
ast/all.hh                                      \
ast/ignores                                     \
ast/visitor.hh                                  \
ast/cloner.hh                                   \
ast/cloner.hxx                                  \
ast/cloner.cc                                   \
ast/default-visitor.hh                          \
ast/default-visitor.hxx                         \
ast/pretty-printer.hh                           \
ast/pretty-printer.cc                           \
$(ast_nodes)

gen_dir = $(top_srcdir)/dev
ast_gen_deps = $(gen_dir)/ast.py \
	       $(gen_dir)/tools.py $(ast_srcdir)/ast.yml

include $(top_srcdir)/dev/ast-gen.mk

dist_libkernel_la_SOURCES +=                    \
ast/all.hh                                      \
ast/declarations-type.hh                        \
ast/declarations-type.cc                        \
ast/exps-type.hh                                \
ast/exps-type.cc                                \
ast/flavor.hh                                   \
ast/flavor.cc                                   \
ast/loc.hh                                      \
ast/nary-fwd.hh                                 \
ast/new-clone.hh                                \
ast/new-clone.hxx                               \
ast/parametric-ast.hh                           \
ast/parametric-ast.hxx                          \
ast/parametric-ast.cc                           \
ast/print.hh ast/print.cc                       \
ast/symbols-type.hh                             \
ast/symbols-type.cc                             \
ast/visitor.hxx                                 \
$(BUILT_SOURCES_ast)
