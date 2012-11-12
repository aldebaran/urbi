# There are problems with ancient versions of convert which fail to
# convert some of our images from PDF to PNG.  In addition, our tool
# chain is handling PNG -> PDF, so having a PDF -> PNG is actually a
# nuisance.
#
# So let's convert here, statically.

PDF2PNG = convert -verbose -density 288x288 -resize 40% -quality 90

FIGS_PDF = $(wildcard */cube-and-disc.pdf */cube-released.pdf */cube.pdf)
FIGS_PNG = $(FIGS_PDF:.pdf=.png)

all: $(FIGS_PDF:.pdf=.png)

.PHONY: clean
clean:
	rm -f $(FIGS_PNG)

%.png: %.pdf
	$(PDF2PNG) $< png:$@.tmp
	mv -f $@.tmp $@
