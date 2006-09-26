m4_pattern_forbid([^ACX_])dnl
m4_pattern_forbid([^URBI_])dnl

AC_PREREQ([2.60])
AC_INIT([kernel1], [1.0])

# Auxiliary files.
AC_CONFIG_AUX_DIR([build-aux])

# Automake.
AM_INIT_AUTOMAKE([1.9.6 foreign -Wall dist-zip])


## ------------ ##
## Parameters.  ##
## ------------ ##

AC_ARG_VAR([OS], [the OS the kernel is compiled for (aibo...)])
AC_CANONICAL_HOST
case $OS in
  "") OS="linux";;
esac
AC_DEFINE_UNQUOTED([OS], [$OS],
	 	   [The OS the kernel is compiled for (aibo...).])

URBI_KERNEL_DIRS([posix])

## --------------- ##
## Documentation.  ##
## --------------- ##

URBI_ARG_PROGS([doxygen], [the Doxygen documentation generation program])
# Name of the directory where the Doxygen documentation is created.
AC_SUBST([DOCDIR], [srcdoc])

AC_CONFIG_FILES([doc/Makefile
		 doc/Doxyfile])

## ------------- ##
## Compilation.  ##
## ------------- ##

AM_PROG_LEX
AC_CONFIG_FILES([build-aux/bison++], [chmod +x build-aux/bison++])
URBI_ARG_PROGS([bison], [the GNU LALR(1) parser generator])
URBI_ARG_PROGS([flex],  [the fast lexical analyzer generator])

# For the time being, we are not interested about deprecated
# headers such as hash_map.h.  We should use ext/hash_map.
AC_SUBST([WARNING_CXXFLAGS], [-Wno-deprecated])
AC_PROG_CXX

# We don't need static libraries, speed the compilation up.
: ${enable_shared=no}
AC_PROG_LIBTOOL


## --------- ##
## Network.  ##
## --------- ##
URBI_OPENR
AM_CONDITIONAL([NETWORK_OPENR],  [$openr])
AM_CONDITIONAL([NETWORK_BSDNET], [true])


## -------- ##
## Output.  ##
## -------- ##

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
