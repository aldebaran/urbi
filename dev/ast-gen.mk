## -------------------------- ##
## Using the AST generators.  ##
## -------------------------- ##

# We use GNU Make extensions.
AUTOMAKE_OPTIONS += -Wno-portability
$(ast_srcdir)/%.stamp: $(gen_dir)/ast-%-gen $(ast_gen_deps)
	@rm -rf $@
	@rm -rf $@.tmp
	@touch $@.tmp
	$(gen_dir)/ast-$*-gen $(ast_srcdir) < $(ast_srcdir)/ast.yml
	@mv -f $@.tmp $@

# ast-nodes.mk
EXTRA_DIST += ast/nodes-mk.stamp
$(ast_srcdir)/ast-nodes.mk: $(ast_srcdir)/nodes-mk.stamp

# all.hh.
EXTRA_DIST += ast/all.stamp
ast/all.hh: $(ast_srcdir)/all.stamp

# fwd.hh.
EXTRA_DIST += ast/fwd.stamp
ast/fwd.hh: $(ast_srcdir)/fwd.stamp

# ignores.
EXTRA_DIST += ast/ignores ast/ignores.stamp
$(ast_srcdir)/ignores.stamp: $(gen_dir)/ast-ignores-gen $(ast_gen_deps)
	@rm -rf $@
	@rm -rf $@.tmp
	@touch $@.tmp
	$(gen_dir)/ast-ignores-gen $(ast_srcdir) < $(ast_srcdir)/ast.yml
	if test -d $(ast_srcdir)/.svn; then \
          svn propset svn:ignore $(ast_srcdir) -F $(ast_srcdir)/ignores; \
        fi
	@mv -f $@.tmp $@
ast/ignores: $(ast_srcdir)/ignores.stamp

# AST itself.
EXTRA_DIST += ast/nodes.stamp
$(ast_nodes): $(ast_srcdir)/nodes.stamp

# ast.dot
$(ast_srcdir)/ast.dot: $(gen_dir)/ast-graph-gen $(ast_gen_deps)
	$(gen_dir)/ast-graph-gen < $(ast_srcdir)/ast.yml >$@.tmp
	mv -f $@.tmp $@

# DefaultVisitor
EXTRA_DIST += ast/default-visitor.stamp
ast/default-visitor.hh ast/default-visitor.hxx: $(ast_srcdir)/default-visitor.stamp

# Visitor.
EXTRA_DIST += ast/visitor.stamp
ast/visitor.hh: $(ast_srcdir)/visitor.stamp

# PrettyPrinter.
EXTRA_DIST += ast/pretty-printer.stamp
ast/pretty-printer.hh ast/pretty-printer.hxx ast/pretty-printer.cc: $(ast_srcdir)/pretty-printer.stamp

MAINTAINERCLEANFILES += $(BUILT_SOURCES_ast)
