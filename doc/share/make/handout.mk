%-handout.tex: %.tex
	perl -pe 's/(documentclass)({.*})/$$1\[handout]$$2/' $< >$@

%-handout-4.pdf: %-handout.pdf
	$(share_bin_dir)/beamer2handout $< 4up $@

CLEANFILES += *-handout-4.pdf
