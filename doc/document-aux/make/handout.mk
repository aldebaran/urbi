%-handout.tex: %.tex
	$(AM_V_GEN) perl -pe 's/(documentclass)({.*})/$$1\[handout]$$2/' $< >$@

# FIXME: We could probably use `pgfpages' instead of beamer2handout,
# according to Beamer's manual (beameruserguide.pdf):
#
#   21.1 Creating Handouts Using the Handout Mode
#
#   [...]
#  
#   When printing a handout created this way, you will typically wish
#   to print at least two and possibly four slides on each page. The
#   easiest way of doing so is presumably to use pgfpages as follows:
#   
#   \usepackage{pgfpages} 
#   \pgfpagesuselayout{2 on 1}[a4paper,border shrink=5mm] 
#   
#   Instead of 2 on 1 you can use 4 on 1 (but then you have to add
#   landscape to the list of options) and you can use, say,
#   letterpaper instead of a4paper.

%-handout-2.pdf: %-handout.pdf
	$(AM_V_GEN) $(share_bin_dir)/beamer2handout $< 2up $@

%-handout-4.pdf: %-handout.pdf
	$(AM_V_GEN) $(share_bin_dir)/beamer2handout $< 4up $@

%-handout-6.pdf: %-handout.pdf
	$(AM_V_GEN) $(share_bin_dir)/beamer2handout $< 6up $@

CLEANFILES += *-handout-2.pdf *-handout-4.pdf *-handout-6.pdf 

# ----------------- #
# Handout + notes.  #
# ----------------- #

%-notes.tex: %.tex
	$(AM_V_GEN) perl \
	  -pe 's/(documentclass)({.*})/$$1\[handout,notes]$$2/' $< >$@

%-notes-2.pdf: %-notes.pdf
	$(AM_V_GEN) $(share_bin_dir)/beamer2handout $< 2up $@

%-notes-4.pdf: %-notes.pdf
	$(AM_V_GEN) $(share_bin_dir)/beamer2handout $< 4up $@

%-notes-6.pdf: %-notes.pdf
	$(AM_V_GEN) $(share_bin_dir)/beamer2handout $< 6up $@

CLEANFILES += *-notes-2.pdf *-notes-4.pdf *-notes-6.pdf 
