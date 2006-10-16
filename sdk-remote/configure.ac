m4_pattern_forbid([^ACX_])dnl
m4_pattern_forbid([^URBI_])dnl

AC_PREREQ([2.60])
AC_INIT([liburbi-c++], [1.0-b2], [bugs@gostai.com],
	[liburbi-c++])

# Auxiliary files.
AC_CONFIG_AUX_DIR([build-aux])

# Automake.
AM_INIT_AUTOMAKE([1.9.6 -Wall dist-zip])
AC_SUBST([INSTALL_HEADER], ['$(SHELL) $(install_sh_DATA) -C'])

# Urbi standard locations.
URBI_DIRS([remote])

URBI_SDK

# Doc generation.
URBI_DOC

## ------------- ##
## Compilation.  ##
## ------------- ##

# For the time being, we are not interested about deprecated
# headers such as hash_map.h.  We should use ext/hash_map.
AC_PROG_CC
URBI_PROG_CXX
# Unfortunately -Weffc++ triggers warnings that cannot be avoided
# when using unions in classes.
TC_CXX_WARNINGS([-Wno-effc++])

# We don't need shared libraries, speed the compilation up.
AC_ENABLE_SHARED([no])
AC_PROG_LIBTOOL

# X11.
AC_PATH_XTRA
AM_CONDITIONAL([X11], [test x"$no_x" != xyes])

# Examples.
AC_ARG_ENABLE([examples],
	      [AC_HELP_STRING([--disable-examples],
			      [do no compile the examples])],
	      [], [enable_examples=yes])
AC_MSG_CHECKING([whether compiling the examples])
AC_MSG_RESULT([$enable_examples])
AM_CONDITIONAL(EXAMPLES, test x"$enable_examples" = xyes)

# Pthreads.
URBI_PTHREAD

# Soundcard.
AC_CHECK_HEADERS([sys/soundcard.h], [soundcard=true], [soundcard=false])
AM_CONDITIONAL([SOUNDCARD], [$soundcard])


# Open R.
URBI_OPENR

## -------- ##
## Output.  ##
## -------- ##

AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_FILES([Makefile
	  src/Makefile
	  src/examples/Makefile
	  src/liburbi/Makefile
	  src/liburbi/uobject/Makefile
	  src/utils/Makefile
	  src/liburbi/openr/Makefile
	  src/liburbi/openr/Makefile.aibo
	  jpeg-6b/makefile.aibo
	  jpeg-6b/Makefile
])
AC_OUTPUT
