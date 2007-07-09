## Process this file with automake --
include $(top_srcdir)/src/kernel.mk
include $(srcdir)/ast-nodes.mk

EXTRA_DIST += ast.yml
BUILT_SOURCES += $(BUILT_SOURCES_local)
# Don't use LOCAL_BUILT_SOURCES since it looks like the SOURCES of the
# program `LOCAL_BUILT' and triggers Automake warnings.
BUILT_SOURCES_local =						\
	fwd.hh							\
	visitor.hh						\
	default-visitor.hh default-visitor.hxx			\
	pretty-printer.hh pretty-printer.hxx pretty-printer.cc	\
	$(AST_NODES)

gen_dir = $(top_srcdir)/dev
ast_gen_deps = $(gen_dir)/ast.py $(gen_dir)/ast_params.py \
	       $(gen_dir)/tools.py $(srcdir)/ast.yml
$(srcdir)/ast-nodes.mk: $(gen_dir)/ast-nodes-mk-gen $(ast_gen_deps)
	$(gen_dir)/ast-nodes-mk-gen < $(srcdir)/ast.yml >$@.tmp
	mv -f $@.tmp $@

## -------------------------- ##
## Using the AST generators.  ##
## -------------------------- ##

# We use GNU Make extensions.
AUTOMAKE_OPTIONS += -Wno-gnu
$(srcdir)/%.stamp: $(gen_dir)/ast-%-gen $(ast_gen_deps)
	@rm -rf $@
	@rm -rf $@.tmp
	@touch $@.tmp
	$(gen_dir)/ast-$*-gen $(srcdir) < $(srcdir)/ast.yml
	@mv -f $@.tmp $@

# fwd.hh.
EXTRA_DIST += fwd.stamp
fwd.hh: $(srcdir)/fwd.stamp

# AST itself.
EXTRA_DIST += nodes.stamp
$(AST_NODES): $(srcdir)/nodes.stamp

$(srcdir)/ast.dot: $(gen_dir)/ast-graph-gen $(ast_gen_deps)
	$(gen_dir)/ast-graph-gen < $(srcdir)/ast.yml >$@.tmp
	mv -f $@.tmp $@

# DefaultVisitor
EXTRA_DIST += default-visitor.stamp
default-visitor.hh default-visitor.hxx: $(srcdir)/default-visitor.stamp

# Visitor.
EXTRA_DIST += visitor.stamp
visitor.hh: $(srcdir)/visitor.stamp

# PrettyPrinter.
EXTRA_DIST += pretty-printer.stamp
pretty-printer.hh pretty-printer.hxx pretty-printer.cc: $(srcdir)/pretty-printer.stamp


MAINTAINERCLEANFILES += $(BUILT_SOURCES_local)

noinst_LTLIBRARIES = libast.la
libast_la_SOURCES =					\
	all.hh                                          \
	loc.hh                                          \
	visitor.hxx					\
	$(BUILT_SOURCES_local)
