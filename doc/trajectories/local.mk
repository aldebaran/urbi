## Copyright (C) 2010, 2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

## -------------- ##
## Trajectories.  ##
## -------------- ##

URBI = $(top_builddir)/tests/bin/urbi$(EXEEXT)
GNUPLOT = gnuplot
$(srcdir)/%.dat: %.utraj
	$(AM_V_GEN)$(ENSURE_TARGET_DIR)
	$(AM_V_at)$(URBI) -q					\
	        -f $(srcdir)/trajectories/plot.u		\
	        -f '$<'						\
	        -e 'plot("$@.tmp", sample(y, 40, 0.1s));'	\
	        -e 'shutdown;'
	$(AM_V_at)mv $@.tmp $@

## A single Make rule with two commands because it simplifies the use:
## depend on one file, not two.
%.pdftex_t %.pdf: %.dat
	$(AM_V_GEN)$(ENSURE_TARGET_DIR)
# gnuplot creates an output, even if it failed.  Remove files from a
# previous run that might have us think it went ok.
	$(AM_V_at)rm -f $*.{tex,eps,pdf,pdftex_t}
# Put the output first, before the plotting command, and before
# requests from the user.  Set the terminal too there for the same
# reasons.
	$(AM_V_at){					\
	  echo 'set output "$*.tex"';			\
	  echo 'set terminal epslatex color';		\
	  echo 'set key off';				\
	  echo 'plot "$<" using 1:2 with linespoints';	\
	} >$*.plt.tmp
	$(AM_V_at)LC_ALL=C $(GNUPLOT) $*.plt.tmp
	$(AM_V_at)mv $*.tex $*.pdftex_t
	$(AM_V_at)$(EPSTOPDF) $*.eps -o $*.pdf
	$(AM_V_at)rm $*.plt.tmp $*.eps


TRAJECTORIES = $(call ls_files,trajectories/*.utraj)
EXTRA_DIST += $(TRAJECTORIES) $(TRAJECTORIES:.utraj=.dat)
CLEANFILES += $(TRAJECTORIES:.utraj=.pdftex_t)
# Not only is this true (i.e., we do want these intermediate files to
# be kept, as we store them in our repository), but it is also
# mandated by GNU Make, who, otherwise, does not seem to understand
# how to go from utraj to pdf with an intermediate step in $(srcdir).
.SECONDARY: $(addprefix $(srcdir)/,$(TRAJECTORIES:.utraj=.dat))

PDF_IMAGES +=					\
  $(TRAJECTORIES:.utraj=.pdf)
