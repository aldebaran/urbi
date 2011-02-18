## Copyright (C) 2008-2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

TESTS += $(EXAMPLES_TESTS)
EXAMPLES_TESTS =

# urbi-send
EXTRA_DIST += examples/urbi-send.m4sh
nodist_check_SCRIPTS += examples/urbi-send
m4sh_scripts += examples/urbi-send

examples/urbi-send.log: examples/urbi-send ../executables.stamp
	@$(am__check_pre) $${dir}$< $(am__check_post)
EXAMPLES_TESTS += examples/urbi-send

# urbi-sendbin
EXTRA_DIST += examples/urbi-sendbin.m4sh
nodist_check_SCRIPTS += examples/urbi-sendbin
m4sh_scripts += examples/urbi-sendbin

examples/urbi-sendbin.log: examples/urbi-sendbin ../executables.stamp
	@$(am__check_pre) $${dir}$< $(am__check_post)
#EXAMPLES_TESTS += examples/urbi-sendbin

# Does not work, I don't know why :(
#TFAIL_TESTS +=  examples/urbi-sendbin

# urbi-image
EXTRA_DIST += examples/urbi-image.m4sh
nodist_check_SCRIPTS += examples/urbi-image
m4sh_scripts += examples/urbi-image

examples/urbi-image.log: examples/urbi-image
	@$(am__check_pre) $${dir}$< $(am__check_post)
EXAMPLES_TESTS += examples/urbi-image
