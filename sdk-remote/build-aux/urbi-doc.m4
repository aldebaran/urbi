m4_pattern_forbid([^ACX_])dnl
m4_pattern_forbid([^URBI_])dnl

AC_PREREQ([2.60])

# URBI_DOC
# --------
AC_DEFUN([URBI_DOC],
[
## --------------- ##
## Documentation.  ##
## --------------- ##

URBI_ARG_PROGS([doxygen], [the Doxygen documentation generation program])
# Name of the directory where the Doxygen documentation is created.
AC_SUBST([DOCDIR], [srcdoc])

AC_CONFIG_FILES([doc/Makefile
		 doc/Doxyfile])
])

## Local Variables:
## mode: autoconf
## End:
