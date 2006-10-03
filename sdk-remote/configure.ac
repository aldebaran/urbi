m4_pattern_forbid([^ACX_])dnl
m4_pattern_forbid([^URBI_])dnl

AC_PREREQ([2.60])
AC_INIT([liburbi-c++], [1.0-b2], [bugs@gostai.com], [liburbi-cpp])

# Auxiliary files.
AC_CONFIG_AUX_DIR([build-aux])

# Automake.
AM_INIT_AUTOMAKE([1.9.6 -Wall dist-zip])

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

# We don't need shared libraries, speed the compilation up.
AC_ENABLE_SHARED([no])
AC_PROG_LIBTOOL

# X11.
AC_PATH_XTRA
AM_CONDITIONAL([X11], [test x"$no_x" != xyes])

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

AC_CONFIG_FILES([Makefile
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
])
AC_OUTPUT
