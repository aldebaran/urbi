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

URBI_KERNEL_DIRS([posix])

URBI_DOC

## ------------- ##
## Compilation.  ##
## ------------- ##

AM_PROG_LEX
AC_CONFIG_FILES([build-aux/bison++], [chmod +x build-aux/bison++])
URBI_ARG_PROGS([bison], [the GNU LALR(1) parser generator])
URBI_ARG_PROGS([flex],  [the fast lexical analyzer generator])

URBI_PROG_CXX

# We don't need static libraries, speed the compilation up.
AC_ENABLE_SHARED([no])
AC_PROG_LIBTOOL


## --------- ##
## Network.  ##
## --------- ##
URBI_OPENR
AM_CONDITIONAL([NETWORK_OPENR],  [$openr])


## -------- ##
## Output.  ##
## -------- ##

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
