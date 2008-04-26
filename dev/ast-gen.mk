## Before loading this file, define ast_basedir.
##
## The base directory to prepend to all the file names generated in the
## generated Makefile snippet.  If this ast-nodes.mk is to be included
## by ast/Makefile.am, then define to empty.  If it is included by its
## parent Makefile.am, define to "ast/".
##
## Likewise, define ast_nodes to the list of all C++ files containing
## the definitions of the AST nodes to be generated.
##
## Using $(ast_srcdir)/foo or $(ast_basedir)foo depends on how the file
## is used.  Makes, including GNU Make although it is better at this
## than the others, do bad things when a file is sometimes referred to
## as "ast/foo", and sometimes "$(srcdir)/ast/foo", even if $(srcdir)
## is ".".  So stick to a single naming scheme.
##
## Below, we always avoid using $(srcdir) when we can.  We need it on
## the *.stamp targets, otherwise they are created in the build tree.
## We need it on "ast-nodes.mk" because it is "included" in which case
## Automake always prepends "$(srcdir)/".


## -------------------------- ##
## Using the AST generators.  ##
## -------------------------- ##

## We use GNU Make extensions.
AUTOMAKE_OPTIONS += -Wno-portability
$(ast_srcdir)/%.stamp: $(gen_dir)/ast-%-gen $(ast_gen_deps)
	@rm -f $@ $@.tmp
	@touch $@.tmp
	$(gen_dir)/ast-$*-gen $(ast_srcdir) < $(ast_srcdir)/ast.yml
	@mv -f $@.tmp $@

## ast-nodes.mk
EXTRA_DIST += $(ast_basedir)nodes-mk.stamp
$(ast_srcdir)/ast-nodes.mk: $(ast_srcdir)/nodes-mk.stamp

## all.hh.
EXTRA_DIST += $(ast_basedir)all.stamp
$(ast_basedir)all.hh: $(ast_srcdir)/all.stamp

## cloner.hh etc.
EXTRA_DIST += $(ast_basedir)cloner.stamp
$(ast_basedir)cloner.hh $(ast_basedir)cloner.cc: $(ast_srcdir)/cloner.stamp

## fwd.hh.
EXTRA_DIST += $(ast_basedir)fwd.stamp
$(ast_basedir)fwd.hh: $(ast_srcdir)/fwd.stamp

## ignores.
EXTRA_DIST += $(ast_basedir)ignores $(ast_basedir)ignores.stamp
$(ast_srcdir)/ignores.stamp: $(gen_dir)/ast-ignores-gen $(ast_gen_deps)
	@rm -f $@ $@.tmp
	@touch $@.tmp
	$(gen_dir)/ast-ignores-gen $(ast_srcdir) < $(ast_srcdir)/ast.yml
	if test -d $(ast_srcdir)/.svn; then \
          svn propset svn:ignore $(ast_srcdir) -F $(ast_srcdir)/ignores; \
	elif test -f $(ast_srcdir)/.gitignore; then \
	  cp $(ast_srcdir)/ignores $(ast_srcdir)/.gitignore; \
        fi
	@mv -f $@.tmp $@
$(ast_basedir)ignores: $(ast_srcdir)/ignores.stamp

## AST itself.
EXTRA_DIST += $(ast_basedir)nodes.stamp
$(ast_nodes): $(ast_srcdir)/nodes.stamp

## ast.dot
EXTRA_DIST += $(ast_basedir)graph.stamp
$(ast_basedir)ast.dot: $(ast_srcdir)/graph.stamp

## DefaultVisitor
EXTRA_DIST += $(ast_basedir)default-visitor.stamp
$(ast_basedir)default-visitor.hh $(ast_basedir)default-visitor.hxx: $(ast_srcdir)/default-visitor.stamp

## Visitor.
EXTRA_DIST += $(ast_basedir)visitor.stamp
$(ast_basedir)visitor.hh: $(ast_srcdir)/visitor.stamp

## PrettyPrinter.
EXTRA_DIST += $(ast_basedir)pretty-printer.stamp
$(ast_basedir)pretty-printer.hh $(ast_basedir)pretty-printer.hxx $(ast_basedir)pretty-printer.cc: $(ast_srcdir)/pretty-printer.stamp

MAINTAINERCLEANFILES += $(BUILT_SOURCES_ast)
