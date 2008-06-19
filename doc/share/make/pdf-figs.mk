# Rules to convert several common figure formats to formats well
# supported by pdflatex (png, jpg, pdf).

# Formats to convert to PDF.
SHARE_EXTS_TO_PDF = \
  dot fig fdp gif id tif tiff pbm pgm ppm pdffig plt $(EXTS_TO_PDF)
# Formats to convert to PNG.
SHARE_EXTS_TO_PNG = dia $(EXTS_TO_PNG)

# FILES
# convert_ext SRC-EXT, DST-EXT, FILEs
# -----------------------------------
# Return the sorted list of $(FILES) with SRC-EXT as extension,
# changing to DST-EXT.
share_convert_ext =					\
   $(sort						\
          $(patsubst %.$(1),%.$(2),			\
                     $(filter %.$(1),$(3))))


# FILES
# convert_exts SRC-EXTS, DST-EXT, FILES
# -------------------------------------
# Map all the extensions in SRC-EXTS to DST-EXT of the $(FILES) list.
share_convert_exts =				\
$(foreach ext,					\
          $(1),					\
          $(call share_convert_ext,$(ext),$(2),$(3)))


# FILES
# share_convert_to_pdf FILES
# --------------------------
# Choose the most appropriate format for PDFLaTeX for the FILES.
# Beware that unknown formats are left out.
share_convert_to_pdf =						\
    $(call share_convert_exts,$(SHARE_EXTS_TO_PDF),pdf,$(1))	\
    $(call share_convert_exts,$(SHARE_EXTS_TO_PNG),png,$(1))


# There seems to be more bugs in dia -> fig -> pdf than dia -> png.
%.png: %.dia
	dia -n -e $@ $<

%.png: %.fdp
	fdp -Tpng $< -o $@


## This does not work properly, especially when the output is bigger
## than A4, in which case ps2epsi crops.
##
## %.eps: %.dot
## 	dot -Gpage=595.842 -Tps2 $< -o $*.ps
## # This line: [ /CropBox [36 36 97 89] /PAGES pdfmark
## # breaks the ps2pdf output.
## 	sed -i '/CropBox/d' $*.ps
## 	ps2epsi $*.ps $@

%.fig: %.dot
	dot -Tfig $< -o $@

%.fig: %.fdp
	fdp -Tfig $< -o $@
## There is a bug in fdp's fig output, see Debian bug 373005.
	perl -0777 -pi -e 's/^2 3 0.*\n.*\n//m' $@

%.pdf: %.fig
	fig2dev -Lpdf -p dummy $< $@

%.eps: %.fig
	fig2dev -Leps -p dummy $< $@


## pdftex/pdftex_t combined XFig pictures.
##
## To avoid problems, the PDF file should end with .pdf, not .pdftex
## as suggested in xfig documentation.  And to avoid ambiguity on
## *.fig -> *.pdf, let's use *.pdffig as input extension instead of
## *.fig.
##
## A single Make rule with two commands because it simplifies the use:
## depend on one file, not two.  Generate the PDF last, since it bears
## the dependency.
%.pdf %.eps %.pdftex_t: %.pdffig
	-rm -f $*.{pdf,pdftex_t}
	fig2dev -Lpdftex_t -p $*.pdf $< $*.pdftex_t
# Some versions of fig2dev produce code for epsfig instead of graphicx.
# Do not force the extension to PDF.
	perl -pi -e 's/\\epsfig\{file=/\\includegraphics{/;' \
		 -e 's/(\\includegraphics\{.*)\.pdf/$$1/;'   \
	  $*.pdftex_t
	fig2dev -Lpdftex    -p dummy $< $*.pdf
	fig2dev -Lpstex    -p dummy $< $*.eps


## pdftex/pdftex_t combined GNU Plot pictures.
##
## The GNU Plot file needs not set the output file name, nor the terminal:
## set are properly set by default.
##
## A single Make rule with two commands because it simplifies the use:
## depend on one file, not two.
%.pdftex_t %.eps %.pdf: %.plt
# gnuplot creates an output, even if it failed.  remove it.
	rm -f $*.{tex,eps,pdf,pdftex_t}
# Put the output first, before the plotting command, and before
# requests from the user.  Set the terminal too there for the same
# reasons.
	{					\
	  echo 'set output "$*.eps"';		\
	  echo 'set terminal epslatex color';	\
	  cat $<;				\
	} > $*.plt.tmp
	LC_ALL=C gnuplot $*.plt.tmp
	mv $*.tex $*.pdftex_t
	epstopdf $*.eps -o $*.pdf
	rm $*.plt.tmp

%.pdf: %.gif
	convert $< $@

%.pdf: %.id
	epstopdf $< -o $@

%.pdf: %.eps
	epstopdf $< -o $@

%.pdf: %.jpg
	convert $< $@

%.pdf: %.png
	convert $< $@

%.pdf: %.tif
	convert $< $@

%.pdf: %.tiff
	convert $< $@

%.pdf: %.pbm
	convert $< $@

%.pdf: %.pgm
	convert $< $@

%.pdf: %.ppm
	convert $< $@
