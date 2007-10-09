ast_srcdir = $(srcdir)/ast
include $(ast_srcdir)/ast-nodes.mk
ast_nodes = $(foreach f,$(AST_NODES),ast/$(f))

EXTRA_DIST += ast/ast.yml
BUILT_SOURCES += $(BUILT_SOURCES_ast)
# Don't use LOCAL_BUILT_SOURCES since it looks like the SOURCES of the
# program `LOCAL_BUILT' and triggers Automake warnings.
BUILT_SOURCES_ast =							   \
	ast/fwd.hh							   \
	ast/all.hh							   \
	ast/ignores							   \
	ast/visitor.hh							   \
	ast/default-visitor.hh ast/default-visitor.hxx			   \
	ast/pretty-printer.hh ast/pretty-printer.hxx ast/pretty-printer.cc \
	$(ast_nodes)

gen_dir = $(top_srcdir)/dev
ast_gen_deps = $(gen_dir)/ast.py $(gen_dir)/ast_params.py \
	       $(gen_dir)/tools.py $(ast_srcdir)/ast.yml


## -------------------------- ##
## Using the AST generators.  ##
## -------------------------- ##

# We use GNU Make extensions.
AUTOMAKE_OPTIONS += -Wno-gnu
$(ast_srcdir)/%.stamp: $(gen_dir)/ast-%-gen $(ast_gen_deps)
	@rm -rf $@
	@rm -rf $@.tmp
	@touch $@.tmp
	$(gen_dir)/ast-$*-gen $(ast_srcdir) < $(ast_srcdir)/ast.yml
	@mv -f $@.tmp $@

# ast-nodes.mk
EXTRA_DIST += nodes-mk.stamp
$(ast_srcdir)/ast-nodes.mk: $(ast_srcdir)/nodes-mk.stamp

# all.hh.
EXTRA_DIST += all.stamp
all.hh: $(ast_srcdir)/all.stamp

# fwd.hh.
EXTRA_DIST += fwd.stamp
fwd.hh: $(ast_srcdir)/fwd.stamp

# ignores.
EXTRA_DIST += ignores ignores.stamp
$(ast_srcdir)/ignores.stamp: $(gen_dir)/ast-ignores-gen $(ast_gen_deps)
	@rm -rf $@
	@rm -rf $@.tmp
	@touch $@.tmp
	$(gen_dir)/ast-ignores-gen $(ast_srcdir) < $(ast_srcdir)/ast.yml
	if test -d $(ast_srcdir)/.svn; then \
          svn propset svn:ignore $(ast_srcdir) -F $(ast_srcdir)/ignores; \
        fi
	@mv -f $@.tmp $@
ignores: $(ast_srcdir)/ignores.stamp

# AST itself.
EXTRA_DIST += nodes.stamp
$(ast_nodes): $(ast_srcdir)/nodes.stamp

# ast.dot
$(ast_srcdir)/ast.dot: $(gen_dir)/ast-graph-gen $(ast_gen_deps)
	$(gen_dir)/ast-graph-gen < $(ast_srcdir)/ast.yml >$@.tmp
	mv -f $@.tmp $@

# DefaultVisitor
EXTRA_DIST += default-visitor.stamp
default-visitor.hh default-visitor.hxx: $(ast_srcdir)/default-visitor.stamp

# Visitor.
EXTRA_DIST += visitor.stamp
visitor.hh: $(ast_srcdir)/visitor.stamp

# PrettyPrinter.
EXTRA_DIST += pretty-printer.stamp
pretty-printer.hh pretty-printer.hxx pretty-printer.cc: $(ast_srcdir)/pretty-printer.stamp


MAINTAINERCLEANFILES += $(BUILT_SOURCES_ast)

dist_libkernel_la_SOURCES +=			\
ast/all.hh					\
ast/loc.hh					\
ast/flavor.hh ast/flavor.cc			\
ast/visitor.hxx

nodist_libkernel_la_SOURCES += 			\
$(BUILT_SOURCES_ast)
