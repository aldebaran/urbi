# Provide the targets "dist", "distdir", and "distcheck".
# The latter only runs "make".
#
# Expects the contents to be shipped to be declared in data_DATA,
# or EXTRA_DIST.

data_DATA += $(EXTRA_DIST)

data_DATA +=	\
share		\
Makefile

distdir ?= $(BASE)
TARBALL ?= $(distdir).tar.gz
dist: distdir
	tar zcf $(TARBALL) $(distdir)
	rm -rf $(distdir)

# File to remove from the tarball.
DIST_IGNORE = \
  -name '*~' -o -name '.svn' -o -name '+committed'
distdir:
	rm -rf $(distdir) $(distdir).tmp.tar
	mkdir $(distdir)
# Use tar, not cp, to preserve the directory parts (cp bar/baz foo/
# yields foo/baz, not foo/bar/baz).  To catch failures (e.g., missing
# files), don't use a pipe to connect the two "tar"s.
	tar cf $(distdir).tmp.tar $(data_DATA)
	cd $(distdir) && tar xf ../$(distdir).tmp.tar
	rm -f $(distdir).tmp.tar
	find $(distdir) $(DIST_IGNORE) | xargs rm -fr

distcheck: dist
	rm -rf _tests
	mkdir _tests
	cd _tests && tar zxf ../$(TARBALL)
	make -C _tests/$(distdir)
	rm -rf _tests

.PHONY: distdir dist distcheck
