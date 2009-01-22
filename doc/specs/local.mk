pdf_DATA += specs/urbi-specs.pdf
urbi_specs_sources = $(call ls_files,specs/*.tex)
urbi_specs_deps = $(urbi_specs_sources) $(call ls_files,*.sty)

# Convert some old LaTeX 2.09 idioms into 2e.
latex2e:
	perl -pi.bak -0777 -e 's/\{\\em (.*?)}/\\emph{$$1}/gs' \
	  $(srcdir)/specs/*.tex

# texi2pdf does not like directories that do not exist.  Don't depend
# on the directory, as only its existence matters, not its time
# stamps.
specs/urbi-specs.pdf: specs/.stamp $(urbi_specs_deps)

specs/.stamp:
	-mkdir specs
	test -f $@ || touch $@

# ## ------- ##
# ## Check.  ##
# ## ------- ##
#
# include $(top_srcdir)/build-aux/check.mk
#
# TESTS = $(urbi_specs_sources)
# # This is not exact, but otherwise it is too long.
# # LAZY_TEST_SUITE = 1
# TEST_LOGS = $(TESTS:.tex=.log)
# # FIXME: Assigned to QH.
# TFAIL_TESTS = specs/lang.tex
#
# URBI_CONSOLE = $(top_builddir)/tests/bin/urbi-console
#
# # Set URBI_PATH to find URBI.INI.
# TESTS_ENVIRONMENT +=				\
#   URBI_CONSOLE=$(URBI_CONSOLE)			\
#   URBI_PATH=$(abs_srcdir)/specs:$$URBI_PATH
#
# TEST_LOGS: $(srcdir)/specs/URBI.INI $(srcdir)/check.py
#
# %.log: %.tex
# 	@$(am__check_pre) $(srcdir)/check.py $${dir}$< $(am__check_post)
