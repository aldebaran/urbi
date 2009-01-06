# specs/local.mk
share_dir = $(srcdir)/document-aux
include document-aux/make/share-am.mk
include document-aux/make/tex.mk

PDFS = specs/urbi-specs.pdf
CLEANFILES += $(PDFS)

all: $(PDFS)

view: $(PDFS)
	xpdf $<

# Convert some old LaTeX 2.09 idioms into 2e.
latex2e:
	perl -pi.bak -0777 -e 's/\{\\em (.*?)}/\\emph{$$1}/gs' \
	  $(srcdir)/specs/*.tex

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

# texi2pdf does not like directories that do not exists.  Don't depend
# on the directory, as only its existence matters, not its time
# stamps.
specs/urbi-specs.pdf: specs/.stamp $(srcdir)/specs/urbi-specs.sty $(srcdir)/specs/indexing.sty $(wildcard $(srcdir)/specs/*.tex)

specs/.stamp:
	-mkdir specs
	test -f $@ || touch $@

URBI_CONSOLE = $(top_builddir)/tests/bin/urbi-console

check:
	for f in $(wildcard *.tex); do				\
	  URBI_CONSOLE=$(URBI_CONSOLE) $(srcdir)/check.py $$f;	\
	done
