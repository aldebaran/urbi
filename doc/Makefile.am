## Copyright (C) 2006, 2008-2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

include $(top_srcdir)/build-aux/make/init.mk
include $(top_srcdir)/build-aux/make/html-dir.mk
include tests/local.mk

# Document aux.
share_dir = $(srcdir)/document-aux
share_bin_dir = $(share_dir)/bin
share_make_dir = $(share_dir)/make
include document-aux/make/tex.mk
include document-aux/make/pdf-figs.mk

# Ship what we need from document-aux.  Something more automatic is
# needed.
EXTRA_DIST +=					\
  bin/api-sort					\
  bin/local-words				\
  bin/toassert					\
  document-aux/bib/comp.lang.urbi.bib		\
  document-aux/bin/texloganalyser		\
  document-aux/bin/texi2dvi			\
  $(call ls_files,				\
    @{document-aux}/figs/gostai.*		\
    @{document-aux}/figs/urbi-sdk/*/cube.*	\
    @{document-aux}/styles/ldf/*		\
    @{document-aux}/overlib/*)			\
  $(cube_and_disc).pdf				\
  $(cube_and_disc).png				\
  document-aux/latex/notation.tex		\
  document-aux/styles/fixme.sty			\
  document-aux/styles/gostai-documentation.sty	\
  document-aux/styles/gostai.sty		\
  document-aux/styles/headings.ist		\
  document-aux/styles/hevea.sty			\
  document-aux/styles/misc.hva			\
  document-aux/styles/misc.sty			\
  document-aux/styles/mycxx.sty			\
  document-aux/styles/myhyperref.sty		\
  document-aux/styles/myifthen.hva		\
  document-aux/styles/myifthen.sty		\
  document-aux/styles/myindex.sty		\
  document-aux/styles/mylistings.hva		\
  document-aux/styles/mylistings.sty		\
  document-aux/styles/texi.hva			\
  document-aux/styles/texi.sty			\
  document-aux/styles/urbi-style.cfg

## -------- ##
## Images.  ##
## -------- ##

# Images that simply used by the CSS of the HTML generated documents.
# So don't convert, just copy into the htmldir directories.
CSS_IMAGES =					\
  $(share_dir)/figs/doc-header.jpg		\
  $(share_dir)/figs/doc-menubarbg.jpg		\
  $(share_dir)/figs/doc-menubarhoverbg.jpg

# Images whose original is in Dot, JPEG, or PNG format.
DOT_IMAGES := $(call ls_files,img/*.dot)
JPG_IMAGES := $(call ls_files,img/*.jpg)
PNG_IMAGES := $(call ls_files,img/*.png)

logodir = document-aux/figs/urbi-sdk/$(PACKAGE_MAJOR).$(PACKAGE_MINOR)
cube_and_disc =	$(logodir)/cube-and-disc
# Images whose original is in PDF.
PDF_IMAGES =							\
  $(call ls_files,img/*.pdf					\
                  @document-aux/figs/urbi-sdk/*/cube.pdf)	\
  document-aux/figs/gostai.pdf					\
  $(cube_and_disc).pdf						\
  $(cubes_pdf)
EXTRA_DIST += $(call ls_files,img/*)

# Images converted to EPS.
EPS_IMAGES =					\
  $(JPG_IMAGES:.jpg=.eps)			\
  $(DOT_IMAGES:.dot=.eps) 			\
  $(PDF_IMAGES:.pdf=.eps) 			\
  $(PNG_IMAGES:.png=.eps)
CONVERT_TO_EPS =				\
  { $(ENSURE_TARGET_DIR) ; } &&			\
  $(CONVERT) $< eps:$@.tmp &&			\
  mv $@.tmp $@

# We do not need conversion from PNG to PDF.  This rule from
# pdf-figs.mk is actually a nuisance, it should probably be removed.
%.pdf: %.png
%.eps: %.pdf
	$(AM_V_GEN)$(CONVERT_TO_EPS)
# We have png => pdf => eps, but the result is poor, and it even fails
# with old versions of ghostscript.
%.eps: %.png
	$(AM_V_GEN)$(CONVERT_TO_EPS)
CLEANFILES += $(DOT_IMAGES:.dot=.pdf) $(EPS_IMAGES)

# Images to include in the documentation directory.
if ENABLE_DOC
imagedir = $(docdir)/images
nodist_image_DATA = $(call ls_files,@$(logodir)/*)
endif

include trajectories/local.mk


## ----------- ##
## LaTeX doc.  ##
## ----------- ##

# Find local *.sty files.  Find converted images from document-aux.
TEXI2DVIFLAGS +=				\
  -I $(srcdir)					\
  -I $(builddir)/document-aux
pdf_DATA =

# Use tex4ht.
TEX4HTFLAGS = urbi-style,2
urbi-sdk-single.htmldir : TEX4HTFLAGS = urbi-style
TEXI2HTMLFLAGS += --tex4ht --tex4ht-options=$(TEX4HTFLAGS) --verbose --debug
TEX4HT_POST = $(abs_top_srcdir)/build-aux/bin/tex4ht-post
MAKEINDEXFLAGS = -s headings.ist
TEX_ENVIRONMENT =				\
  PATH=$$PATH:$(abspath $(share_bin_dir))	\
  MAKEINDEX="makeindex $(MAKEINDEXFLAGS)"	\
  TEXINDY="texindy $(TEXINDYFLAGS)"		\
  TEX4HT="tex4ht -cunihtf -utf8"

%.htmldir: %.tex $(share_tex_dependencies) $(share_dir)/styles/urbi-style.cfg
	$(AM_V_GEN)rm -rf $@ $@.tmp $@.log
	$(AM_V_at)$(mkdir_p) $@.tmp || true
	$(AM_V_at)							\
	$(if $(TEX_ENVIRONMENT), export $(TEX_ENVIRONMENT);)		\
	if $(TEXI2HTML) $(TEXI2HTMLFLAGS) -o $@.tmp/$(*F).html $<;	\
	then								\
	  test ! -f $@.log ||						\
	    $(TEXLOGANALYZER) $(TEXLOGANALYZERFLAGS) $@.log >&2;	\
	else								\
	  sta=$$?;							\
	  test ! -f $@.log ||						\
	    cat >&2 $@.log;						\
	  exit $$sta;							\
	fi
	$(AM_V_at)cd $@.tmp					\
	  && $(TEX4HT_POST) --index $(*F).html $(*F)*.html
	$(AM_V_at)							\
	  cp $(srcdir)/$(cube_and_disc).png  $@.tmp/$(*F)0x.png;	\
	  cp $(share_dir)/figs/gostai.png    $@.tmp/$(*F)1x.png;	\
	  case "$(*F)" in						\
	    (urbi-sdk*)							\
	    cp $(srcdir)/img/urbi-architecture.png $@.tmp/$(*F)2x.png;;	\
	  esac
##	$(AM_V_at)case ',$(TEX4HTFLAGS),' in
##	  (*,mouseover,*) cp $(share_dir)/overlib/* $@.tmp;;
##	esac
	$(AM_V_at)cp $(CSS_IMAGES) $(share_dir)/overlib/* $@.tmp
	$(AM_V_at)mv $@.tmp $@
CLEANFILES += $(MASTERS:.tex=.htmldir.log)


## -------------- ##
## urbi-sdk.pdf.  ##
## -------------- ##

# All the documents that can be compiled.  Used only for dependencies.
MASTERS =					\
  urbi-naming.tex				\
  urbi-sandbox.tex				\
  urbi-sdk-single.tex				\
  urbi-sdk.tex

PDFS =						\
  urbi-naming.pdf				\
  urbi-sdk.pdf
EXTRA_DATA += $(PDFS)
if ENABLE_DOC_PDF
  pdf_DATA += $(PDFS)
endif

# We cannot simply use html_DATA here, since Automake does not
# support installing directories.
CLEANDIRS += $(MASTERS:.tex=.htmldir)
if ENABLE_DOC_HTML
  html_DIR += urbi-sdk.htmldir
endif

CLEANFILES += $(pdf_DATA)
urbi_sdk_tex := $(call ls_files,*.tex)
# Do _not_ include revision.sty in the sources, as it is continiously
# updated.  It would cause endless recompilations.
urbi_sdk_sources =				\
  $(urbi_sdk_tex)				\
  $(filter-out revision.sty,			\
    $(call ls_files,*.sty tables/*.bnf tables/*.txt))

CLEANFILES += config.sty


## -------------- ##
## revision.sty.  ##
## -------------- ##

# The tricky part is that all our PDF and HTML outputs depend on
# revision.sty, so it is tempting to put it as a regular prerequisite.
# This way we are sure it exists when we need it.  But to ensure that
# it is always up to date (so that the PDF and HTML output are always
# referring to the revision info at the time of "make" run) we have to
# make "revision.sty" a PHONY target, i.e., a target fired at each
# "make" invocation.
#
# But then we have a problem: "make" will notice that revision.sty is
# to be remade, and therefore that the PDF and HTML have to be remade
# at each run of "make".  No way!
#
# We could remove the dependency on revision.sty from our PDF and
# HTML, and start the rules with "$(MAKE) update-revision".  But this
# is not good for concurrent builds, as we would have several PDF
# and/or HTML documents that try to update the same file,
# concurrently.
#
# The simplest solution seems to be using "order-only prerequisites"
# (specified with "|", not ":").  In that case Make knows that if the
# PDF and revision.sty are to be updated, them update the latter
# first.  Yet, it is not because revision.sty is updated that we need
# to update the PDF files.  And of course, Make handles itself the
# concurrency issues.

# Don't depend on .version, it changes too often.
REVISION_FILE = revision.sty
BUILT_SOURCES += $(REVISION_FILE)
.PHONY: $(REVISION_FILE)
$(REVISION_FILE):
	$(AM_V_GEN)$(VERSIONIFY)			\
	  --directory --cache=$(top_srcdir)/.version	\
	  --latex=$@

## ------------------------ ##
## Generated source files.  ##
## ------------------------ ##

# Generate in the source tree and keep under revision control, so that
# we don't have to require from tarball users that they have to run
# maintainer tools such as tex2chk.  Not to mention that there are
# then many nasty issues arising with src=build builds.
FROM_GEN =					\
  guide/requirements.tex
guide/requirements.tex: $(srcdir)/guide/build.tex
BUILT_SOURCES += $(FROM_GEN)
EXTRA_DIST += $(FROM_GEN:=.gen)

$(FROM_GEN): %: %.gen
	$(AM_V_GEN)mkdir -p $(@D)
	$(AM_V_at)srcdir=$(srcdir) $< > $@.tmp
	$(AM_V_at)chmod a-w $@.tmp
	$(AM_V_at)$(move_if_change_run) $@.tmp $@
	$(AM_V_at)touch $@


EXTRA_DIST += $(urbi_sdk_sources)
urbi_sdk_deps =					\
  config.sty					\
  $(FROM_GEN)					\
  $(urbi_sdk_sources)

$(MASTERS:.tex=.pdf): $(urbi_sdk_deps) $(PDF_IMAGES)	\
                    | $(REVISION_FILE)
urbi-naming.pdf urbi-sdk.pdf: $(DOT_IMAGES:.dot=.pdf)
$(MASTERS:.tex=.dvi) $(MASTERS:.tex=.htmldir): $(urbi_sdk_deps) $(EPS_IMAGES) \
                                             | $(REVISION_FILE)

view: $(pdf_DATA)
	xpdf $<

# Compile urbi-sandbox, to see if some construct works well.
.PHONY: sandbox
sandbox: urbi-sandbox.htmldir urbi-sandbox.pdf
	open urbi-sandbox.pdf
	open urbi-sandbox.htmldir/index.html

## ------------------------------- ##
## Additional text documentation.  ##
## ------------------------------- ##

# Always install these guys.  At least because our build and package
# process uses them (especially COMPONENTS.txt).
dist_doc_DATA   = AUTHORS.txt LICENSE.txt THANKS.txt
nodist_doc_DATA = COMPONENTS.txt README.txt
CLEANFILES += $(nodist_doc_DATA)

## ---------------- ##
## Spell-checking.  ##
## ---------------- ##

.PHONY: spell-check
SPELL_CHECK = $(abs_top_srcdir)/build-aux/bin/spell-check
AM_SPELL_CHECKFLAGS = -p $(srcdir)/urbi.dict --clean
EXTRA_DIST += $(build_aux_dir)/bin/spell-check urbi.dict
RUN_SPELL_CHECK = $(SPELL_CHECK) $(AM_SPELL_CHECKFLAGS) $(SPELL_CHECKFLAGS)
# Rely on Make to resolve the location (src or build) of the files to
# spell-check.
spell-check: $(urbi_sdk_tex) $(dist_doc_DATA) $(nodist_doc_DATA)
	$(AM_V_GEN) $(RUN_SPELL_CHECK) $(if $(V:0=),-v,-q) $^


# A program whose output is FILE:SECTIONING COMMANDS.
# To make easy via grep to control the structure.
latex_subsections =							\
  perl -ne '/(.*?):\\subsection({.*})/ && { $$secs{$$1} .= $$2 };'	\
       -e  'END { print "$$_:$$secs{$$_}\n" foreach sort keys %secs; }'
# This is not spell-checking, but that's easier for the BF.
# An order-only dependency so that we don't get style-check in the
# list of dependencies of spell-check (we would try to find errors in
# the style-check non-existing file).
.PHONY spell-check: | style-check
style-check: $(urbi_sdk_tex)
## Use the {verbatim} environment, not {lstlisting}.
	$(AM_V_GEN) ! grep '{lstlisting}' $^
## Never indent verbatim environments, it leaves empty lines.
	$(AM_V_at) ! grep -E \
	  '^  *\\(begin|end){(cxx|java|urbi(comment|script|unchecked)|verbatim)}' $^
## The stdlib sections must follow a strict pattern.
##	$(AM_V_at) ! grep -E '^\\subsection\{' $^ |		\
##	  grep '/specs/' |					\
##	  grep -Ev '(lang|naming-standard|ros|tools).tex' |	\
##	  $(latex_subsections) |				\
##	  grep -Ev '^.*:({Examples?})?.*{Prototypes}({Construction})?{Slots}$$'

## --------- ##
## install.  ##
## --------- ##

# Make the *.txt documentation more accessible.  Cannot be handled in
# script/post-install since it is launched at the end of install-exec
# in the top-level diretory, which is processed before this `doc'
# directory.
install-data-hook:
	if test -n "$(DESTDIR)"; then		\
	  cd "$(DESTDIR)$(prefix)" &&		\
	  for i in share/doc/urbi-sdk/*.txt; do	\
	    case $(URBI_HOST_OS) in		\
	      (windows)				\
	        unix2dos "$$i" &&		\
	        cp -f "$$i" .			\
	        ;;				\
	      (*)				\
	        $(LN_S) -f "$$i"		\
	        ;;				\
	    esac				\
	  done					\
	fi


## ------------- ##
## install-doc.  ##
## ------------- ##
.PHONY: install-doc
doc_files =					\
  $(pdf_DATA)					\
  urbi-sdk.htmldir urbi-sdk-single.htmldir	\
  images					\
  $(top_srcdir)/scripts/www/doc/HEADER.html
images: $(nodist_image_DATA)
	$(AM_V_GEN)rm -rf $@ $@.tmp
	$(AM_V_at)mkdir -p $@.tmp
	$(AM_V_at)cp $^ $@.tmp
	$(AM_V_at)mv $@.tmp $@
install-doc: $(doc_files)
	$(AM_V_GEN)rsync $(RSYNCFLAGS) $(doc_files) $(doc_host):$(doc_dir)

