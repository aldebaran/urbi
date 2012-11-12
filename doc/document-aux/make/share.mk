# Automake does not want the user to touch some targets (not even
# giving new dependencies).  clean and others are such targets.  So in
# order to make share/ useful with Automake too, we must not define
# these targets.  Hence a two-layer system: share.mk is to be included
# by regular Make users, and share-am.mk to be used by Automake users.
share_dir ?= share

AM_DEFAULT_VERBOSITY ?= 0
AM_V_GEN = $(am__v_GEN_$(V))
am__v_GEN_ = $(am__v_GEN_$(AM_DEFAULT_VERBOSITY))
am__v_GEN_0 = @echo "  GEN   " $@;
AM_V_at = $(am__v_at_$(V))
am__v_at_ = $(am__v_at_$(AM_DEFAULT_VERBOSITY))
am__v_at_0 = @

include $(share_dir)/make/share-am.mk
mostlyclean: mostlyclean-local
clean: clean-local mostlyclean
	rm -f $(CLEANFILES)
.PHONY: mostlyclean mostlyclean-local clean clean-local
