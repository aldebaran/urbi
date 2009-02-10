# Yes, this is not by the book, but it is so cooler.
dist_urbi_DATA := $(call ls_files,share/urbi/*.u)

package_info_u = $(top_srcdir)/share/urbi/package-info.u
nodist_urbi_DATA = $(package_info_u)
$(package_info_u): $(top_srcdir)/.version
	$(top_srcdir)/build-aux/git-version-gen		\
		--cache=$<				\
		--srcdir=$(top_srcdir)			\
		--urbi --output=$@

$(srcdir)/share/urbi/tutorial-content.u: share/urbi/tutorial/tutorial.xml share/urbi/tutorial/tutorial.py
	rm -f $@ $@.tmp
	$(srcdir)/share/urbi/tutorial/tutorial.py $< > $@.tmp
	mv $@.tmp $@
