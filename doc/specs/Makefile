share_dir=document-aux
include document-aux/make/share.mk
include document-aux/make/tex.mk

PDFS = urbi-specs.pdf
CLEANFILES += $(PDFS)

all: $(PDFS)

view: $(PDFS)
	xpdf $<

latex2e:
	perl -pi.bak -0777 -e 's/\{\\em (.*?)}/\\emph{$$1}/gs' *.tex

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

urbi-specs.pdf: urbi-specs.sty float.tex list.tex
