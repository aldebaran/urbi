# Automake does not want the user to touch some targets (not even
# giving new dependencies).  clean and others are such targets.  So in
# order to make share/ useful with Automake too, we must not define
# these targets.  Hence a two-layer system: share.mk is to be included
# by regular Make users, and share-am.mk to be used by Automake users.
share_dir ?= share
include $(share_dir)/make/share-am.mk
mostlyclean: mostlyclean-local
clean: clean-local mostlyclean
	rm -f $(CLEANFILES)
.PHONY: mostlyclean mostlyclean-local clean clean-local
