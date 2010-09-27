include $(top_srcdir)/build-aux/init.mk
# include $(top_srcdir)/build-aux/doxygen.mk
include $(top_srcdir)/build-aux/revision.mk
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
  document-aux/bin/texi2dvi			\
  $(call ls_files,@{document-aux}/figs/* 	\
	          @{document-aux}/styles/ldf/*)	\
  document-aux/styles/fixme.sty			\
  document-aux/styles/gostai-documentation.sty	\
  document-aux/styles/gostai-report.sty		\
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
  document-aux/styles/texi.sty


## -------- ##
## Images.  ##
## -------- ##

DOT_IMAGES = $(call ls_files,img/*.dot)
PNG_IMAGES =							\
  $(call ls_files,img/*.png					\
                  @{../modules/system/ros}/doc/img/*.png)

cube_and_disc = urbi-sdk/$(PACKAGE_MAJOR).$(PACKAGE_MINOR)/cube-and-disc
PDF_IMAGES =					\
  $(call ls_files,img/*.pdf)			\
  document-aux/figs/gostai.pdf			\
  document-aux/figs/$(cube_and_disc).pdf	\
  $(DOT_IMAGES:.dot=.pdf)
EXTRA_DIST += $(call ls_files,img/*)

EPS_IMAGES = $(PDF_IMAGES:.pdf=.eps) $(PNG_IMAGES:.png=.eps)
CONVERT_TO_EPS =				\
  { $(ENSURE_TARGET_DIR) ; } &&			\
  $(CONVERT) $< eps:$@.tmp &&			\
  mv $@.tmp $@

%.eps: %.pdf
	$(CONVERT_TO_EPS)
# We have png => pdf => eps, but the result is poor, and it even fails
# with old versions of ghostscript.
%.eps: %.png
	$(CONVERT_TO_EPS)
CLEANFILES += $(EPS_IMAGES)


# Find local *.sty files.  Find converted images from document-aux.
TEXI2DVIFLAGS +=				\
  -I $(srcdir)					\
  -I $(builddir)/document-aux			\
  -I $(top_srcdir)/modules/system/ros/doc	\
  -I $(top_builddir)/modules/system/ros/doc
pdf_DATA =

# Use tex4ht.
TEX4HTFLAGS = urbi-style,2,sections+
TEXI2HTMLFLAGS += --tex4ht --verbose --tex4ht-options=$(TEX4HTFLAGS)
TEX4HT_POST = $(abs_top_srcdir)/build-aux/bin/tex4ht-post
%.htmldir: %.tex $(share_tex_dependencies) urbi-style.cfg
	-rm -rf $@ $@.tmp
	-$(mkdir_p) $@.tmp
	$(TEXI2HTML) $(TEXI2HTMLFLAGS) -o $@.tmp/$(notdir $*).html $<
	cd $@.tmp && $(TEX4HT_POST) -v -v --index $(notdir $*).html $(notdir $*)*.html
# We need to find a better means to provide our own *.png here.  I
# have not used the name "gostai.png" because stupidly the PDF
# compilation seems to prefer *.png to *.pdf, which is wrong here.
	cp $(share_dir)/figs/$(cube_and_disc).png $@.tmp/urbi-sdk0x.png
	cp $(share_dir)/figs/gostai.png $@.tmp/urbi-sdk1x.png
	mv $@.tmp $@


## ------------------------------- ##
## Additional text documentation.  ##
## ------------------------------- ##

dist_doc_DATA =					\
  AUTHORS.txt					\
  LICENSE.txt					\
  README.txt					\
  THANKS.txt

## -------------- ##
## urbi-sdk.pdf.  ##
## -------------- ##

# All the documents that can be compiled.  Used only for dependencies.
MASTERS =					\
  urbi-naming.tex				\
  urbi-sandbox.tex				\
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
if ENABLE_DOC_HTML
  html_DIR += urbi-sdk.htmldir
endif

CLEANFILES += $(pdf_DATA)
urbi_sdk_tex = $(call ls_files,*.tex)
urbi_sdk_sources =				\
  $(urbi_sdk_tex)				\
  $(call ls_files,*.sty tables/*.txt)
REVISION_FILE = revision.sty
REVISIONFLAGS = --latex
EXTRA_DIST += $(urbi_sdk_sources)
urbi_sdk_deps = 				\
  config.sty					\
  $(REVISION_FILE)				\
  $(urbi_sdk_sources)

$(MASTERS:.tex=.pdf): $(urbi_sdk_deps) $(PDF_IMAGES)
$(MASTERS:.tex=.dvi) $(MASTERS:.tex=.htmldir): $(urbi_sdk_deps) $(EPS_IMAGES)

view: $(pdf_DATA)
	xpdf $<

# Compile urbi-sandbox, to see if some construct works well.
.PHONY: sandbox
sandbox: urbi-sandbox.htmldir urbi-sandbox.pdf
	open urbi-sandbox.pdf
	open urbi-sandbox.htmldir/index.html

include trajectories/local.mk


## ---------------- ##
## Spell-checking.  ##
## ---------------- ##

.PHONY: spell-check
SPELL_CHECK = $(abs_top_srcdir)/build-aux/bin/spell-check
AM_SPELL_CHECKFLAGS = -p urbi.dict --clean
EXTRA_DIST += $(build_aux_dir)/bin/spell-check
RUN_SPELL_CHECK = $(SPELL_CHECK) $(AM_SPELL_CHECKFLAGS) $(SPELL_CHECKFLAGS)
spell-check:
	cd $(srcdir) &&						\
	$(RUN_SPELL_CHECK) $(urbi_sdk_tex) $(dist_doc_DATA)


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
doc_files = $(pdf_DATA) urbi-sdk.htmldir
install-doc: $(doc_files)
	scp -r $(doc_files) $(doc_host):$(doc_dir)/doc.new
