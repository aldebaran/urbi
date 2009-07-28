TESTS += $(EXAMPLES_TESTS)
EXAMPLES_TESTS =

# urbi-send
EXTRA_DIST += examples/urbi-send.m4sh
nodist_check_SCRIPTS += examples/urbi-send
m4sh_scripts += examples/urbi-send

examples/urbi-send.log: examples/urbi-send
	@$(am__check_pre) $${dir}$< $(am__check_post)
EXAMPLES_TESTS += examples/urbi-send

# urbi-sendbin
EXTRA_DIST += examples/urbi-sendbin.m4sh
nodist_check_SCRIPTS += examples/urbi-sendbin
m4sh_scripts += examples/urbi-sendbin

examples/urbi-sendbin.log: examples/urbi-sendbin
	@$(am__check_pre) $${dir}$< $(am__check_post)
#EXAMPLES_TESTS += examples/urbi-sendbin

# Does not work, I don't know why :(
#TFAIL_TESTS +=  examples/urbi-sendbin
