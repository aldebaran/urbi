# urbi-send
EXTRA_DIST += examples/urbi-send.as
nodist_check_SCRIPTS += examples/urbi-send
m4sh_scripts += examples/urbi-send

examples/urbi-send.log: examples/urbi-send
	@$(am__check_pre) $${dir}$< $(am__check_post)

EXAMPLES_TESTS = examples/urbi-send
TESTS += $(EXAMPLES_TESTS)
