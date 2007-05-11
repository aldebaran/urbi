## Process this file with automake --
include $(top_srcdir)/src/kernel.mk
include $(srcdir)/ast-nodes.mk

EXTRA_DIST += ast.yml
BUILT_SOURCES += $(BUILT_SOURCES_local)
# Don't use LOCAL_BUILT_SOURCES since it looks like the SOURCES of the
# program `LOCAL_BUILT' and triggers Automake warnings.
BUILT_SOURCES_local =				\
	fwd.hh					\
	visitor.hh				\
	loc.hh                                  \
	$(AST_NODES)

gen_dir = $(top_srcdir)/dev
ast_gen_deps = $(gen_dir)/ast.py $(gen_dir)/tools.py $(srcdir)/ast.yml
$(srcdir)/ast-nodes.mk: $(gen_dir)/ast-nodes-mk-gen $(ast_gen_deps)
	$(gen_dir)/ast-nodes-mk-gen < $(srcdir)/ast.yml >$@.tmp
	mv -f $@.tmp $@

EXTRA_DIST += ast-fwd-gen.stamp
$(srcdir)/ast-fwd-gen.stamp: $(gen_dir)/ast-fwd-gen $(ast_gen_deps)
	@rm -rf $@
	@rm -rf $@.tmp
	@touch $@.tmp
	$(gen_dir)/ast-fwd-gen $(srcdir) < $(srcdir)/ast.yml
	@mv -f $@.tmp $@
fwd.hh: ast-fwd-gen.stamp

EXTRA_DIST += ast-visitor-gen.stamp
$(srcdir)/ast-visitor-gen.stamp: $(gen_dir)/ast-visitor-gen $(ast_gen_deps)
	@rm -rf $@
	@rm -rf $@.tmp
	@touch $@.tmp
	$(gen_dir)/ast-visitor-gen $(srcdir) < $(srcdir)/ast.yml
	@mv -f $@.tmp $@
visitor.hh: ast-visitor-gen.stamp

EXTRA_DIST += ast-nodes-gen.stamp
$(srcdir)/ast-nodes-gen.stamp: $(gen_dir)/ast-nodes-gen $(ast_gen_deps)
	@rm -rf $@
	@rm -rf $@.tmp
	@touch $@.tmp
	$(gen_dir)/ast-nodes-gen $(srcdir) < $(srcdir)/ast.yml
	@mv -f $@.tmp $@
$(AST_NODES): ast-nodes-gen.stamp

$(srcdir)/ast.dot: $(gen_dir)/ast-graph-gen $(ast_gen_deps)
	$(gen_dir)/ast-graph-gen < $(srcdir)/ast.yml >$@.tmp
	mv -f $@.tmp $@

MAINTAINERCLEANFILES += $(BUILT_SOURCES_local)

noinst_LTLIBRARIES = libast.la
libast_la_SOURCES =					\
	all.hh                                          \
	fwd.hh						\
	visitor.hh					\
	loc.hh                                          \
	$(AST_NODES)					\
	visitor.hxx					\
	default-visitor.hh default-visitor.hxx
