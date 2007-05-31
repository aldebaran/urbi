## Process this file with automake --
include $(top_srcdir)/src/kernel.mk
include $(srcdir)/ast-nodes.mk

EXTRA_DIST += ast.yml
BUILT_SOURCES += $(BUILT_SOURCES_local)
# Don't use LOCAL_BUILT_SOURCES since it looks like the SOURCES of the
# program `LOCAL_BUILT' and triggers Automake warnings.
BUILT_SOURCES_local =					\
	fwd.hh						\
	visitor.hh					\
	pretty-printer.hh pretty-printer.cc		\
	$(AST_NODES)

gen_dir = $(top_srcdir)/dev
ast_gen_deps = $(gen_dir)/ast.py $(gen_dir)/ast_params.py \
	       $(gen_dir)/tools.py $(srcdir)/ast.yml
$(srcdir)/ast-nodes.mk: $(gen_dir)/ast-nodes-mk-gen $(ast_gen_deps)
	$(gen_dir)/ast-nodes-mk-gen < $(srcdir)/ast.yml >$@.tmp
	mv -f $@.tmp $@

# fwd.hh.
EXTRA_DIST += fwd.stamp
$(srcdir)/fwd.stamp: $(gen_dir)/ast-fwd-gen $(ast_gen_deps)
	@rm -rf $@
	@rm -rf $@.tmp
	@touch $@.tmp
	$(gen_dir)/ast-fwd-gen $(srcdir) < $(srcdir)/ast.yml
	@mv -f $@.tmp $@
fwd.hh: fwd.stamp

# AST itself.
EXTRA_DIST += ast.stamp
$(srcdir)/ast.stamp: $(gen_dir)/ast-nodes-gen $(ast_gen_deps)
	@rm -rf $@
	@rm -rf $@.tmp
	@touch $@.tmp
	$(gen_dir)/ast-nodes-gen $(srcdir) < $(srcdir)/ast.yml
	@mv -f $@.tmp $@
$(AST_NODES): ast.stamp

$(srcdir)/ast.dot: $(gen_dir)/ast-graph-gen $(ast_gen_deps)
	$(gen_dir)/ast-graph-gen < $(srcdir)/ast.yml >$@.tmp
	mv -f $@.tmp $@

# Visitor.
EXTRA_DIST += visitor.stamp
$(srcdir)/visitor.stamp: $(gen_dir)/ast-visitor-gen $(ast_gen_deps)
	@rm -rf $@
	@rm -rf $@.tmp
	@touch $@.tmp
	$(gen_dir)/ast-visitor-gen $(srcdir) < $(srcdir)/ast.yml
	@mv -f $@.tmp $@
visitor.hh: visitor.stamp

# PrettyPrinter.
EXTRA_DIST += pretty-printer.stamp
$(srcdir)/pretty-printer.stamp: $(gen_dir)/ast-pretty-printer-gen $(ast_gen_deps)
	@rm -rf $@
	@rm -rf $@.tmp
	@touch $@.tmp
	$(gen_dir)/ast-pretty-printer-gen $(srcdir) < $(srcdir)/ast.yml
	@mv -f $@.tmp $@
pretty-printer.hh pretty-printer.cc: pretty-printer.stamp


MAINTAINERCLEANFILES += $(BUILT_SOURCES_local)

noinst_LTLIBRARIES = libast.la
libast_la_SOURCES =					\
	all.hh                                          \
	loc.hh                                          \
	visitor.hxx					\
	default-visitor.hh default-visitor.hxx		\
	$(BUILT_SOURCES_local)
