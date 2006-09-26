# URBI_OPENR
# ----------
# Look for the OpenR SDK.
AC_DEFUN([URBI_OPENR],
[OPENR_SDK_PATH=/usr/local/OPEN_R_SDK
AC_ARG_WITH([openr],
	    [AC_HELP_STRING([--with-openr=sdk-path], [Turn on OPENR client])],
	    [], [openr=false])

case $with_openr in
   no) openr=false;;
    *) openr=true
       if test "$with_openr" != "yes"; then
	 OPENR_SDK_PATH=$with_openr
       fi;;
esac


# checking if openr sdk is realy there
AC_MSG_CHECKING([for openr SDK])
if test $openr != false; then
  if test -f $OPENR_SDK_PATH/bin/mipsel-linux-c++; then
    AC_MSG_RESULT([$OPENR_SDK_PATH])
  else
    AC_MSG_RESULT([no, mipsel c++ compiler not found in $OPENR_SDK_PATH/bin])
    openr=false
  fi
else
  AC_MSG_RESULT([no])
fi
AC_SUBST([OPENR_SDK_PATH])

AM_CONDITIONAL([OPENR], [$openr])
])


## Local Variables:
## mode: autoconf
## End:
