## -------------- ##
## Trajectories.  ##
## -------------- ##

URBI = $(top_builddir)/tests/bin/urbi
GNUPLOT = gnuplot
$(srcdir)/%.dat: %.utraj
	$(ENSURE_TARGET_DIR)
	$(URBI) -q						\
	        -f $(srcdir)/trajectories/plot.u		\
	        -f '$<'						\
	        -e 'plot("$@.tmp", sample(y, 40, 0.1s));'	\
	        -e 'shutdown;'
	mv $@.tmp $@

## pdftex/pdftex_t combined GNU Plot pictures.
##
## The GNU Plot file needs not set the output file name, nor the terminal:
## set are properly set by default.
##
## A single Make rule with two commands because it simplifies the use:
## depend on one file, not two.
%.pdftex_t %.eps %.pdf: %.dat
	$(ENSURE_TARGET_DIR)
# gnuplot creates an output, even if it failed.  Remove it.
	rm -f $*.{tex,eps,pdf,pdftex_t}
# Put the output first, before the plotting command, and before
# requests from the user.  Set the terminal too there for the same
# reasons.
	{						\
	  echo 'set output "$*.eps"';			\
	  echo 'set terminal epslatex color';		\
	  echo 'set key off';				\
	  echo 'plot "$<" using 1:2 with linespoints';	\
	} > $*.plt.tmp
	LC_ALL=C $(GNUPLOT) $*.plt.tmp
	mv $*.tex $*.pdftex_t
	$(EPSTOPDF) $*.eps -o $*.pdf
	rm $*.plt.tmp


TRAJECTORIES = $(call ls_files,trajectories/*.utraj)
EXTRA_DIST += $(TRAJECTORIES) $(TRAJECTORIES:.utraj=.dat)

# Not only is this true (i.e., we do want these intermediate files to
# be kept, as we store them in our repository), but it is also
# mandated by GNU Make, who, otherwise, does not seem to understand
# how to go from utraj to pdf with an intermediate step in $(srcdir).
.SECONDARY: $(addprefix $(srcdir)/,$(TRAJECTORIES:.utraj=.dat))

PDF_IMAGES +=					\
  $(TRAJECTORIES:.utraj=.pdf)
