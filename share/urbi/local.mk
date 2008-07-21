# Yes, this is not by the book, but it is so cooler.
dist_urbi_DATA := $(wildcard $(srcdir)/share/urbi/*.u)

$(srcdir)/share/urbi/tutorial-content.u: share/urbi/tutorial/tutorial.xml share/urbi/tutorial/tutorial.py
	rm -f $@ $@.tmp
	$(srcdir)/share/urbi/tutorial/tutorial.py $< > $@.tmp
	mv $@.tmp $@
