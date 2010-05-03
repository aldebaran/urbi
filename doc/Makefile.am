include $(top_srcdir)/build-aux/init.mk
include $(top_srcdir)/build-aux/doxygen.mk
include $(top_srcdir)/build-aux/revision.mk
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
  document-aux/figs/gostai.pdf			\
  document-aux/figs/gostai.png			\
  document-aux/figs/urbi.pdf			\
  document-aux/figs/urbi.png			\
  document-aux/styles/fixme.sty			\
  document-aux/styles/gostai-documentation.sty	\
  document-aux/styles/gostai-report.sty		\
  document-aux/styles/hevea.sty			\
  document-aux/styles/ldf/asf.sty		\
  document-aux/styles/ldf/asn1.sty		\
  document-aux/styles/ldf/aterm.sty		\
  document-aux/styles/ldf/bibtex.sty		\
  document-aux/styles/ldf/bnf.sty		\
  document-aux/styles/ldf/box.sty		\
  document-aux/styles/ldf/c++.sty		\
  document-aux/styles/ldf/d.sty			\
  document-aux/styles/ldf/dot.sty		\
  document-aux/styles/ldf/grammar.sty		\
  document-aux/styles/ldf/hir.sty		\
  document-aux/styles/ldf/leopard.sty		\
  document-aux/styles/ldf/lex.sty		\
  document-aux/styles/ldf/mips.sty		\
  document-aux/styles/ldf/nemerle.sty		\
  document-aux/styles/ldf/parse-testsuite.sty	\
  document-aux/styles/ldf/prism.sty		\
  document-aux/styles/ldf/reactive-modules.sty	\
  document-aux/styles/ldf/rtg.sty		\
  document-aux/styles/ldf/scool.sty		\
  document-aux/styles/ldf/sdf.sty		\
  document-aux/styles/ldf/stl.sty		\
  document-aux/styles/ldf/stratego.sty		\
  document-aux/styles/ldf/tiger.sty		\
  document-aux/styles/ldf/treecc.sty		\
  document-aux/styles/ldf/urbiscript.sty	\
  document-aux/styles/ldf/vaucanson.sty		\
  document-aux/styles/ldf/yacc.sty		\
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
PNG_IMAGES = $(call ls_files,img/*.png)
PDF_IMAGES =					\
  $(call ls_files,img/*.pdf)			\
  document-aux/figs/gostai.pdf			\
  document-aux/figs/urbi.pdf			\
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
TEXI2DVIFLAGS += -I $(srcdir) -I $(builddir)/document-aux
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
	cp $(share_dir)/figs/urbi.png   $@.tmp/urbi-sdk0x.png
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

PDFS =						\
  urbi-naming.pdf				\
  urbi-sdk.pdf
EXTRA_DATA += $(PDFS)
if ENABLE_DOC_PDF
  pdf_DATA += $(PDFS)
endif

# We cannot simply use html_DATA here, since Automake does not
# support installing directories.
html_DIR = urbi-sdk.htmldir
EXTRA_html_DIR = urbi-sandbox.htmldir
install-data-hook: install-html-dir
.PHONY: install-html-dir
if ENABLE_DOC_HTML
noinst_DATA += $(html_DIR)
install-html-dir: $(html_DIR)
	$(mkdir_p) $(DESTDIR)$(htmldir)
	tar cf - $(html_DIR) | (cd $(DESTDIR)/$(htmldir) && tar xf -)
endif

CLEANFILES += $(pdf_DATA)
urbi_sdk_sty = $(call ls_files,*.sty)
urbi_sdk_tex = $(call ls_files,*.tex)
# The license files.
urbi_sdk_txt = $(call ls_files,tables/*.txt)
urbi_sdk_sources = $(urbi_sdk_sty) $(urbi_sdk_tex) $(urbi_sdk_txt)
REVISION_FILE = revision.sty
REVISIONFLAGS = --latex
EXTRA_DIST += $(urbi_sdk_sources)
urbi_sdk_deps = 				\
  $(REVISION_FILE)				\
  $(urbi_sdk_sources)

urbi-sandbox.pdf $(pdf_DATA) $(pdf_DATA:.pdf=.dvi) $(html_DIR) $(EXTRA_html_DIR): $(urbi_sdk_deps)
urbi-sandbox.pdf $(pdf_DATA): $(PDF_IMAGES)
$(pdf_DATA:.pdf=.dvi) $(html_DIR) $(EXTRA_html_DIR): $(EPS_IMAGES)

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
EXTRA_DIST += $(top_srcdir)/build-aux/bin/spell-check
RUN_SPELL_CHECK = $(SPELL_CHECK) $(AM_SPELL_CHECKFLAGS) $(SPELL_CHECKFLAGS)
spell-check:
	cd $(srcdir) &&						\
	$(RUN_SPELL_CHECK) $(urbi_sdk_tex) $(dist_doc_DATA)


## --------- ##
## install.  ##
## --------- ##

# Make the *.txt documentation more accessible.
install-data-hook:
	cd "$(DESTDIR)$(prefix)" &&		\
	for i in share/doc/urbi-sdk/*.txt; do	\
	  case $(URBI_HOST_OS) in		\
	    (windows)				\
	      unix2dos "$$i" &&			\
	      cp -f "$$i" .			\
	      ;;				\
	    (*)					\
	      $(LN_S) -f "$$i"			\
	      ;;				\
	  esac					\
	done



## ------------- ##
## install-doc.  ##
## ------------- ##
.PHONY: install-doc
doc_host = downloads@www.gostai.com
doc_dir = gostai.com-downloads/$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)
doc_files = $(pdf_DATA) $(html_DIR)
install-doc: $(doc_files)
	ssh $(doc_host) 'rm -rf $(doc_dir)/doc.new && \
			 mkdir -p $(doc_dir)/doc.new'
	scp -r $(doc_files) $(doc_host):$(doc_dir)/doc.new
	ssh $(doc_host)				\
	  'cd $(doc_dir) &&			\
	   if test -d doc; then			\
	     bak=$$(date +"%F-%T") &&		\
	     {					\
	       mv doc doc.$$bak &&		\
	       mv doc.new doc;			\
	     } ||				\
	    mv doc.$$bak doc;			\
	  else					\
	    mv doc.new doc;			\
	  fi'
