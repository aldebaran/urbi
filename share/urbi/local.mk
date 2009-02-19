# Yes, this is not by the book, but it is so cooler.
dist_urbi_DATA := $(call ls_files,share/urbi/*.u)

package_info_u = share/urbi/package-info.u
nodist_urbi_DATA = $(package_info_u)
REVISIONFLAGS = --urbi
REVISION_FILE = $(package_info_u)
include $(top_srcdir)/build-aux/revision.mk

$(srcdir)/share/urbi/tutorial-content.u: share/urbi/tutorial/tutorial.xml share/urbi/tutorial/tutorial.py
	rm -f $@ $@.tmp
	$(srcdir)/share/urbi/tutorial/tutorial.py $< > $@.tmp
	mv $@.tmp $@

urbi.stamp: $(dist_urbi_DATA) $(package_info_u)
	echo "$$(date)"': $? changed' >>$@

