include $(top_srcdir)/build-aux/init.mk
include $(top_srcdir)/build-aux/doxygen.mk
include $(top_srcdir)/build-aux/revision.mk
include tests/local.mk

# Document aux.
share_dir = $(top_srcdir)/doc/document-aux
share_bin_dir = $(share_dir)/bin
share_make_dir = $(share_dir)/make
include document-aux/make/tex.mk

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

# Find local *.sty files, workaround some strange behavior of texi2dvi
# that wants to play tricks with ~ (which becomes a tilda, instead of
# a nonbreakable space).
TEXI2DVI_FLAGS += -I $(srcdir) -~
pdf_DATA =

# Use tex4ht.
TEXI2HTML_FLAGS += --tex4ht --verbose --debug --tex4ht-options=urbi-style,2,sections+
%.htmldir: %.tex $(share_tex_dependencies)
	-rm -rf $@ $@.tmp
	-$(mkdir_p) $@.tmp
	$(TEXI2HTML) $(TEXI2HTML_FLAGS) -o $@.tmp/$(notdir $*).html $<
# We need to find a better means to provide our own *.png here.  I
# have not used the name "gostai.png" because stupidly the PDF
# compilation seems to prefer *.png to *.pdf, which is wrong here.
	cp $(share_dir)/figs/urbi.png   $@.tmp/urbi-sdk0x.png
	cp $(share_dir)/figs/gostai.png $@.tmp/urbi-sdk1x.png
	cd $@.tmp && $(LN_S) $(notdir $*).html index.html
	mv $@.tmp $@

include specs/local.mk
include tutorial/local.mk

## ------------------------------- ##
## Additional text documentation.  ##
## ------------------------------- ##

dist_doc_DATA =					\
  LICENSE.txt					\
  README.txt					\
  RELEASE-NOTES.txt				\
  REPORTING-BUGS.txt


## -------------- ##
## urbi-sdk.pdf.  ##
## -------------- ##

pdf_DATA +=					\
  urbi-faq.pdf					\
  urbi-naming.pdf				\
  urbi-sdk.pdf
# We cannot simply use html_DATA here, since Automake does not
# support installing directories.
html_DIR = urbi-sdk.htmldir
noinst_DATA += $(html_DIR)
install-data-hook: install-html-dir
install-html-dir: $(html_DIR)
	$(mkdir_p) $(DESTDIR)$(htmldir)
	tar cf - $(html_DIR) | (cd $(DESTDIR)/$(htmldir) && tar xf -)

CLEANFILES += $(pdf_DATA)
urbi_sdk_sources = $(call ls_files,*.tex)
REVISION_FILE = revision.sty
REVISIONFLAGS = --latex
EXTRA_DIST += $(urbi_sdk_sources)
urbi_sdk_deps = 				\
  $(REVISION_FILE)				\
  $(call ls_files,*.sty *.cls)			\
  $(urbi_sdk_sources)

$(pdf_DATA): $(PDF_IMAGES)
urbi-sdk.dvi $(html_DIR) $(pdf_DATA): $(urbi_sdk_deps)

urbi-sdk.dvi $(html_DIR): $(EPS_IMAGES)

view: $(pdf_DATA)
	xpdf $<

%.eps: %.pdf
	mkdir -p $(dir $@) || true
	$(CONVERT) "$<" "$@"

%.eps: %.png
	mkdir -p $(dir $@) || true
	$(CONVERT) "$<" "$@"

FIG2DEV = fig2dev
%.pdf: %.dot
	mkdir -p $(dir $@) || true
# Some versions of dot don't support PDF output.
	if $(DOT) -Tpdf </dev/null 2>/dev/null; then	\
	  $(DOT) -Tpdf $< -o $@.tmp &&			\
	  $(PDFCROP) $@.tmp $@ &&			\
	  rm $@.tmp;					\
	else						\
	  $(DOT) -Tfig $< -o $*.fig &&			\
	  $(FIG2DEV) -Lpdf -p dummy $*.fig $@ &&	\
	  rm $*.fig;					\
	fi


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
