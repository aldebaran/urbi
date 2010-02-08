## -------------- ##
## Trajectories.  ##
## -------------- ##

URBI = $(top_builddir)/tests/bin/urbi
GNUPLOT = gnuplot
$(srcdir)/%.dat: %.utraj
	$(URBI) -f $(srcdir)/trajectories/plot.u	\
	     -f '$<'					\
	     -e 'plot("$@.tmp", sample(y, 40, 0.1s));'	\
	     -e 'shutdown;'
	mv $@.tmp $@

%.pdf: %.dat
	$(GNUPLOT) -e 'set terminal pdf;'			\
	        -e 'plot "$<" using 1:2 with linespoints'	\
	        >$@.tmp
	mv $@.tmp $@


TRAJECTORIES = $(call ls_files,trajectories/*.utraj)

# Not only is this true (i.e., we do want these intermediate files to
# be kept, as we store them in our repository), but it is also
# mandated by GNU Make, who, otherwise, does not seem to understand
# how to go from utraj to pdf with an intermediate step in $(srcdir).
.SECONDARY: $(addprefix $(srcdir)/,$(TRAJECTORIES:.utraj=.dat))

PDF_IMAGES +=					\
  $(TRAJECTORIES:.utraj=.pdf)
