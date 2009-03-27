include $(top_srcdir)/build-aux/init.mk
include $(top_srcdir)/build-aux/doxygen.mk

# Document aux.
share_dir = $(srcdir)/document-aux
share_bin_dir = $(share_dir)/bin
share_make_dir = $(share_dir)/make
include document-aux/make/tex.mk
EXTRA_DIST +=								   \
  $(addprefix document-aux/,$(call ls_files_in_dir,$(srcdir)/document-aux))

# img/.
EXTRA_DIST += $(call ls_files,img/*.pdf)

# Find urbi-report.sty.
TEXI2DVI_FLAGS += -I $(srcdir)
pdf_DATA =

include specs/local.mk
include tutorial/local.mk
include tests/local.mk

## ------------------------------- ##
## Additional text documentation.  ##
## ------------------------------- ##

dist_doc_DATA =					\
  LICENSE.txt					\
  QUICK-START.txt				\
  README.txt					\
  RELEASE-NOTES.txt				\
  REPORTING-BUGS.txt

## ------------ ##
## urbi-sdk.pdf ##
## ------------ ##

pdf_DATA += urbi-sdk.pdf
urbi_sdk_sources = $(call ls_files,*.tex)
urbi_sdk_deps = $(urbi_sdk_sources) $(call ls_files,*.sty)
EXTRA_DIST += $(urbi_sdk_deps) $(urbi_sdk_deps)
urbi-sdk.pdf: $(urbi_sdk_deps)
debug:
	echo $(urbi_sdk_deps)

CLEANFILES += $(pdf_DATA)

view: $(pdf_DATA)
	xpdf $<

# Convert some old LaTeX 2.09 idioms into 2e.
latex2e:
	perl -pi.bak -0777 -e 's/\{\\em (.*?)}/\\emph{$$1}/gs' \
	  $(srcdir)/specs/*.tex


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
