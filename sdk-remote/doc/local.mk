## Copyright (C) 2010-2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

include $(top_srcdir)/build-aux/make/doxygen.mk
include $(top_srcdir)/build-aux/make/html-dir.mk

## ----- ##
## Doc.  ##
## ----- ##

%.dox: %.dox.in
	cd $(top_builddir) && $(SHELL) ./config.status sdk-remote/$@

# Avoid to fire the compilation of Doxygen documentation when we
# modify something which is not part of our sources.
doc/sdk-remote.htmldir: $(call ls_files, *.hh *.hxx *.cc)

# We cannot simply use html_DATA here, since Automake does not
# support installing directories.
if ENABLE_DOC_DOXYGEN
html_DIR += doc/sdk-remote.htmldir
endif

## ------ ##
## Java.  ##
## ------ ##

if BINDING_JAVA
# Take into account Java sources and generated sources, since the
# latter include most of the Doxygen documentation.
#
# Specify at least one generated file (urbi.java, also covered by the
# wildcard) so that the build would fail if the files were not
# generated.
JAVA_FILES =							\
  $(call ls_files, src/swig/java/urbi/*.java)			\
  $(sdk_remote_builddir)/src/swig/java/urbi/urbi.java		\
  $(wildcard $(sdk_remote_builddir)/src/swig/java/urbi/*.java)
doc/sdk-remote-java.htmldir: $(JAVA_FILES)
if ENABLE_DOC_DOXYGEN
html_DIR += doc/sdk-remote-java.htmldir
endif ENABLE_DOC_DOXYGEN
endif BINDING_JAVA

## ------------- ##
## install-doc.  ##
## ------------- ##
.PHONY: install-doc
doc_files =					\
  doc/sdk-remote.htmldir			\
  doc/sdk-remote-java.htmldir
install-doc: $(doc_files)
	$(AM_V_GEN)rsync $(RSYNCFLAGS) $(doc_files) $(doc_host):$(doc_dir)
