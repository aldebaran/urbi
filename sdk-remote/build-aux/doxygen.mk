## To use this module, DOCDIR must be defined.

## ----------------------- ##
## Doxygen documentation.  ##
## ----------------------- ##

EXTRA_DIST = $(srcdir)/$(DOCDIR)
$(srcdir)/$(DOCDIR): Doxyfile.in
	rm -rf $(DOCDIR).tmp $(srcdir)/$(DOCDIR)
	$(MAKE) $(AM_MAKEFLAGS) Doxyfile
	$(DOXYGEN) Doxyfile
	mv $(DOCDIR).tmp $(srcdir)/$(DOCDIR)

maintainer-clean-local:
	rm -rf $(DOCDIR).tmp $(srcdir)/$(DOCDIR)

# We install by hand, otherwise Automake produces "install .../srcdoc
# .../srcdoc", which install our dir into the previous one, instead of
# replacing it.
install-data-local:
	rm -rf $(DESTDIR)$(htmldir)/$(DOCDIR)
	$(mkdir_p) $(DESTDIR)$(htmldir)
	cp -r $(srcdir)/$(DOCDIR) $(DESTDIR)$(htmldir)

uninstall-local:
	chmod -R 700 $(DESTDIR)$(htmldir)/$(DOCDIR)
	rm -rf $(DESTDIR)$(htmldir)/$(DOCDIR)
