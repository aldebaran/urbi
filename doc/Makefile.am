include $(top_srcdir)/build-aux/init.mk
include $(top_srcdir)/build-aux/doxygen.mk
include $(top_srcdir)/build-aux/revision.mk
include tests/local.mk

# Document aux.
share_dir = $(top_srcdir)/doc/document-aux
share_bin_dir = $(share_dir)/bin
share_make_dir = $(share_dir)/make
include document-aux/make/tex.mk
include document-aux/make/pdf-figs.mk

# img/.
DOT_IMAGES = $(call ls_files,img/*.dot)
PNG_IMAGES = $(call ls_files,img/*.png)
PDF_IMAGES =					\
  $(call ls_files,img/*.pdf)			\
  $(share_dir)/figs/gostai.pdf			\
  $(share_dir)/figs/urbi.pdf			\
  $(DOT_IMAGES:.dot=.pdf)
EXTRA_DIST += $(call ls_files,img/*)
EPS_IMAGES = $(PDF_IMAGES:.pdf=.eps) $(PNG_IMAGES:.png=.eps)
CLEANFILES += $(EPS_IMAGES)

# Find local *.sty files.
TEXI2DVI_FLAGS += -I $(srcdir)
pdf_DATA =

# Use tex4ht.
TEX4HT_FLAGS = urbi-style,2,sections+
TEXI2HTML_FLAGS += --tex4ht --verbose --tex4ht-options=$(TEX4HT_FLAGS)
TEX4HT_POST = $(abs_top_srcdir)/build-aux/bin/tex4ht-post
%.htmldir: %.tex $(share_tex_dependencies) urbi-style.cfg
	-rm -rf $@ $@.tmp
	-$(mkdir_p) $@.tmp
	$(TEXI2HTML) $(TEXI2HTML_FLAGS) -o $@.tmp/$(notdir $*).html $<
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
  LICENSE.txt					\
  README.txt

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
urbi_sdk_sources = $(call ls_files,*.tex)
REVISION_FILE = revision.sty
REVISIONFLAGS = --latex
EXTRA_DIST += $(urbi_sdk_sources)
urbi_sdk_deps = 				\
  $(REVISION_FILE)				\
  $(call ls_files,*.sty *.cls)			\
  $(urbi_sdk_sources)

urbi-sandbox.pdf $(pdf_DATA) $(pdf_DATA:.pdf=.dvi) $(html_DIR) $(EXTRA_html_DIR): $(urbi_sdk_deps)
urbi-sandbox.pdf $(pdf_DATA): $(PDF_IMAGES)
$(pdf_DATA:.pdf=.dvi) $(html_DIR) $(EXTRA_html_DIR): $(EPS_IMAGES)

view: $(pdf_DATA)
	xpdf $<

%.eps: %.pdf
	mkdir -p $(dir $@) || true
	$(CONVERT) "$<" "$@"

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
EXTRA_DIST += $(SPELL_CHECK)
spell-check:
	cd $(srcdir) &&						\
	$(SPELL_CHECK) -p urbi.dict $(urbi_sdk_sources) $(dist_doc_DATA)


## --------- ##
## install.  ##
## --------- ##

# Made the *.txt documentation more accessible.
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
doc_dir = gostai.com-downloads/urbi-sdk-2.0
doc_files = $(pdf_DATA) $(html_DIR)
install-doc: $(doc_files)
	ssh $(doc_host) 'rm -rf $(doc_dir)/doc.new && \
			 mkdir -p $(doc_dir)/doc.new'
	scp -r $(doc_files) $(doc_host):$(doc_dir)/doc.new
	ssh $(doc_host)				\
	  'bak=$$(date +"%F-%T") &&		\
	   cd $(doc_dir) &&			\
	   { 					\
	     mv doc doc.$$bak &&		\
	     mv doc.new doc;			\
	   } || mv doc.$$bak doc'

## -------------- ##
## document-aux.  ##
## -------------- ##

# Once these works properly, will be moved into share/ to complete the
# svn-only version.
share-up:
	cd $(share_dir) && git pull --rebase
	git commit -m "Update $(share_dir)." $(share_dir)

share-ci:
	cd $(share_dir) && git commit -v -a
	cd $(share_dir) && git pull --rebase
	cd $(share_dir) && git push origin master
	git commit -m "Update $(share_dir)." $(share_dir)
