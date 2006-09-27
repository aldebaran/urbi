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

## --------------- ##
## Documentation.  ##
## --------------- ##

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
URBI_PTHREAD

# Soundcard.
AC_CHECK_HEADERS([sys/soundcard.h], [soundcard=true], [soundcard=false])
AM_CONDITIONAL([SOUNDCARD], [$soundcard])

# Open R.
URBI_OPENR

AC_CONFIG_FILES([umake/umake], [chmod +x umake/umake])

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
])
AC_OUTPUT
