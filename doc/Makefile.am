include $(top_srcdir)/build-aux/init.mk
include $(top_srcdir)/build-aux/doxygen.mk

# specs/local.mk
share_dir = $(srcdir)/document-aux
share_bin_dir = $(share_dir)/bin
share_make_dir = $(share_dir)/make
include document-aux/make/tex.mk

pdf_DATA =

include specs/local.mk
include tutorial/local.mk
include tests/local.mk

CLEANFILES += $(pdf_DATA)

view: $(pdf_DATA)
	xpdf $<

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

