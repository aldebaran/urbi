## Copyright (C) 2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

include build-aux/make/doxygen.mk
include build-aux/make/html-dir.mk

## ----- ##
## Doc.  ##
## ----- ##

# Avoid to fire the compilation of Doxygen documentation when we
# modify something which is not part of our sources.
doc/sdk-remote.htmldir: $(call ls_files, *.hh *.hxx *.cc)

# We cannot simply use html_DATA here, since Automake does not
# support installing directories.
if ENABLE_DOC_DOXYGEN
html_DIR += doc/sdk-remote.htmldir
endif

## ------------- ##
## install-doc.  ##
## ------------- ##
.PHONY: install-doc
doc_files = doc/sdk-remote.htmldir
install-doc: $(doc_files)
	scp -r $(doc_files) $(doc_host):$(doc_dir)/doc.new
