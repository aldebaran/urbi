include $(top_srcdir)/build-aux/init.mk
include $(top_srcdir)/build-aux/doxygen.mk
include $(top_srcdir)/build-aux/revision.mk

# Document aux.
share_dir = $(srcdir)/document-aux
share_bin_dir = $(share_dir)/bin
share_make_dir = $(share_dir)/make
include document-aux/make/tex.mk
EXTRA_DIST +=								   \
  $(addprefix document-aux/,$(call ls_files_in_dir,$(srcdir)/document-aux))

# img/.
PDF_IMAGES = $(call ls_files,img/*.pdf)
EXTRA_DIST += $(PDF_IMAGES) $(call ls_files,img/*.png)
EPS_IMAGES = $(PDF_IMAGES:.pdf=.eps)
CLEANFILES += $(EPS_IMAGES)

# Our system leaves trailing unused *chk files.
CHECK_CLEANFILES +=						\
  $(shell find $(srcdir) -name '*.chk' -or -name 'test.chk')

# Find urbi-report.sty, and workaround some strange behavior of
# texi2dvi that wants to play tricks with ~ (which becomes a tilda,
# instead of a nonbreakable space).
TEXI2DVI_FLAGS += -I $(srcdir) -~
pdf_DATA =

# Use tex4ht
TEXI2HTML_FLAGS += --tex4ht --verbose --debug

include specs/local.mk
include tutorial/local.mk
include tests/local.mk

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

pdf_DATA += urbi-sdk.pdf
CLEANFILES += $(pdf_DATA)
urbi_sdk_sources = $(call ls_files,*.tex)
REVISION_FILE = revision.sty
REVISIONFLAGS = --latex
urbi_sdk_deps = $(urbi_sdk_sources) $(call ls_files,*.sty) $(REVISION_FILE)
EXTRA_DIST += $(urbi_sdk_sources)
urbi-sdk.dvi urbi-sdk.html urbi-sdk.pdf: $(urbi_sdk_deps)
urbi-sdk.dvi urbi-sdk.html: $(EPS_IMAGES)

view: $(pdf_DATA)
	xpdf $<

%.eps: %.pdf
	-mkdir -p $(dir $@)
	convert $< $@


## -------------- ##
## document-aux.  ##
## -------------- ##
.PHONY: install-doc
html_files = 					\
  urbi-sdk.css					\
  urbi-sdk.html					\
  urbi-sdk0x.png				\
  urbi-sdk1x.png				\
  urbi-sdk2.html				\
  urbi-sdk3.html				\
  urbi-sdk4.html				\
  urbi-sdk5.html
install-doc: $(pdf_DATA) urbi-sdk.html
doc_host = downloads@www.gostai.com
doc_dir = gostai.com-downloads/urbi-sdk-2.0
doc_files = $(pdf_DATA) $(html_files) 
# We need to find a better means to provide our own *.png here.  I
# have not used the name "gostai.png" because stupidly the PDF
# compilation seems to prefer *.png to *.pdf, which is wrong here.
	cp $(srcdir)/img/urbi-sdk1x.png urbi-sdk1x.png
	ssh $(doc_host) 'rm -rf $(doc_dir)/doc.new && \
			 mkdir -p $(doc_dir)/doc.new'
	scp $(doc_files) $(doc_host):$(doc_dir)/doc.new
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
