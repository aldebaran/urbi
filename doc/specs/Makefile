share_dir=document-aux
include document-aux/make/share.mk
include document-aux/make/tex.mk

PDFS = urbi-specs.pdf
CLEANFILES += $(PDFS)

all: $(PDFS)

view: $(PDFS)
	xpdf $<

urbi-specs.pdf: urbi-specs.sty float.tex list.tex
