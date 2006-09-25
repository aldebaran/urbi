m4_pattern_forbid([^ACX_])dnl
m4_pattern_forbid([^URBI_])dnl

AC_PREREQ([2.60])
AC_INIT([liburbi-c++], [0.9.3a], [bugs@gostai.com], [liburbi-cpp])

# Auxiliary files.
AC_CONFIG_AUX_DIR([build-aux])

# Automake.
AM_INIT_AUTOMAKE([1.9.6 -Wall dist-zip])

# Urbi standard locations.
URBI_DIRS([remote])

## --------------- ##
## Documentation.  ##
## --------------- ##

# URBI_ARG_PROGS(PROG, DOC, [PROGRAMS])
# -------------------------------------
# Look for PROG, registered as an ARG_VAR with DOC, and store in
# $PROG (upper cased).  If not available, set as "missing PROG".
m4_define([URBI_ARG_PROGS],
[AC_ARG_VAR(m4_toupper([$1]), [$2])
AC_CHECK_PROGS(m4_toupper([$1]), [m4_default([$3], [$1])],
	       [$am_aux_dir/missing $1])
])

URBI_ARG_PROGS([doxygen], [the Doxygen documentation generation program])
# Name of the directory where the Doxygen documentation is created.
AC_SUBST([DOCDIR], [srcdoc])

AC_CONFIG_FILES([doc/Doxyfile])

## ------------- ##
## Compilation.  ##
## ------------- ##

# For the time being, we are not interested about deprecated
# headers such as hash_map.h.  We should use ext/hash_map.
AC_SUBST([WARNING_CXXFLAGS], [-Wno-deprecated])
AC_PROG_CC
AC_PROG_CXX

# We don't need static libraries, speed the compilation up.
: ${enable_shared=no}
AC_PROG_LIBTOOL

# Pthreads.
ACX_PTHREAD([windows=false], [
    AC_CHECK_HEADER(windows.h, [LIBS="$LIBS -lws2_32 -lgdi32";windows=true],
      [AC_MSG_ERROR([pthreads or WIN32 API is required])])])
AM_CONDITIONAL([WIN32], [test $windows = true])

# Soundcard.
AC_CHECK_HEADERS([sys/soundcard.h], [soundcard=true], [soundcard=false])
AM_CONDITIONAL([SOUNDCARD], [$soundcard])


## -------- ##
## Open R.  ##
## -------- ##

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


## -------- ##
## Output.  ##
## -------- ##

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
