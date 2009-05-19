# Yes, this is not by the book, but it is so cooler.
dist_urbi_DATA := $(call ls_files,share/urbi/*.u)

nodist_urbi_DATA =				\
  $(package_info_u)				\
  share/urbi/platform.u				\
  share/urbi/tutorial-content.u

# package-info.u
package_info_u = share/urbi/package-info.u
REVISIONFLAGS = --urbi
REVISION_FILE = $(package_info_u)
include $(top_srcdir)/build-aux/revision.mk

# tutorial-content.u.
tutorial_sources =				\
  share/urbi/tutorial/tutorial.xml		\
  share/urbi/tutorial/tutorial.py
EXTRA_DIST += $(tutorial_sources)
$(srcdir)/share/urbi/tutorial-content.u: $(tutorial_sources)
	rm -f $@ $@.tmp
	$(srcdir)/share/urbi/tutorial/tutorial.py $< > $@.tmp
	mv $@.tmp $@

urbi.stamp: $(dist_urbi_DATA) $(nodist_urbi_DATA)
	@echo "$$(date)"': $?' >>$@
