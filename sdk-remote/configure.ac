m4_pattern_forbid([^ACX_])dnl

AC_PREREQ([2.59])
AC_INIT([liburbi-c++], [0.9.3a])

# Auxiliary files.
AC_CONFIG_AUX_DIR([build-aux])

# Automake.
AM_INIT_AUTOMAKE([1.9.6 -Wall dist-zip])

# For the time being, we are not interested about deprecated
# headers such as hash_map.h.  We should use ext/hash_map.
AC_SUBST([WARNING_CXXFLAGS], [-Wno-deprecated])
AC_PROG_CXX

AC_PROG_CC

# We don't need static libraries, speed the compilation up.
: ${enable_shared=no}
AC_PROG_LIBTOOL

# Pthreads.
ACX_PTHREAD([], [AC_MSG_ERROR([pthreads are required])])

OPENR_SDK_PATH=/usr/local/OPEN_R_SDK
AC_ARG_ENABLE([openr],
 [AC_HELP_STRING([--enable-openr=sdk_path], [Turn on OPENR client])],
 [case "${enableval}" in
    no|false)  openr=false;;
    *) openr=true
       if test  ${enableval} != "yes" ; then
	 OPENR_SDK_PATH=${enableval}
       fi;;
  esac],
  [openr=false])

# removed sh -c "cd lib/jpeg-6b && ./configure"
# checking if openr sdk is realy there
echo -n "checking for openr SDK... "
if test $openr != false; then
  if test ! -f $OPENR_SDK_PATH/bin/mipsel-linux-c++; then
    echo -e "no\n ##mipsel c++ compiler not found in $OPENR_SDK_PATH/bin/, disabling openr"
    openr=false
  else
    echo $OPENR_SDK_PATH
  fi
fi

AM_CONDITIONAL([OPENR], [test $openr = true])
AC_SUBST([OPENR_SDK_PATH])

AC_CONFIG_FILES([Makefile
	  doc/Makefile
	  lib/Makefile
	  src/Makefile
	  src/examples/Makefile
	  src/liburbi/Makefile
	  src/liburbi/uobject/Makefile
	  src/utils/Makefile
	  src/liburbi/openr/Makefile
	  src/liburbi/openr/Makefile.aibo
	  lib/jpeg-6b/makefile.aibo
	  lib/jpeg-6b/Makefile
	  sdk/Makefile
	  sdk/Makefile.param
])
AC_OUTPUT
