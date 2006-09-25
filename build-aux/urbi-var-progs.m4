# URBI_ARG_PROGS(PROG, DOC, [PROGRAMS])
# -------------------------------------
# Look for PROG, registered as an ARG_VAR with DOC, and store in
# $PROG (upper cased).  If not available, set as "missing PROG".
AC_DEFUN([URBI_ARG_PROGS],
[AC_ARG_VAR(m4_toupper([$1]), [$2])
AC_CHECK_PROGS(m4_toupper([$1]), [m4_default([$3], [$1])],
	       [$am_aux_dir/missing $1])
])


## Local Variables:
## mode: Autoconf
## End:
