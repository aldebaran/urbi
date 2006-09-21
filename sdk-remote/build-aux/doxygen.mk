## To use this module, DOCDIR must be defined.

## ----------------------- ##
## Doxygen documentation.  ##
## ----------------------- ##

html_DATA = $(srcdir)/$(DOCDIR)
$(srcdir)/$(DOCDIR): Doxyfile.in
	rm -rf $(DOCDIR).tmp $(srcdir)/$(DOCDIR)
	$(MAKE) $(AM_MAKEFLAGS) Doxyfile
	$(DOXYGEN) Doxyfile
	mv $(DOCDIR).tmp $(srcdir)/$(DOCDIR)

install-data-local:
	rm -rf $(DESTDIR)$(htmldir)/$(DOCDIR)
	$(mkdir_p) $(DESTDIR)$(htmldir)
	cp -r $(srcdir)/$(DOCDIR) $(DESTDIR)$(htmldir)

maintainer-clean-local:
	rm -rf $(DOCDIR).tmp $(srcdir)/$(DOCDIR)
