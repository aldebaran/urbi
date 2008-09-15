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

urbi-specs.pdf: urbi-specs.sty float.tex list.tex
