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

URBI_KERNEL_DIRS([engine])

URBI_DOC

## ------------- ##
## Compilation.  ##
## ------------- ##

AC_CONFIG_FILES([build-aux/bison++], [chmod +x build-aux/bison++])
TC_PROG([bison], [>= 2.2],   [BISON], [GNU parser generator])
TC_PROG([flex],  [== 2.5.4], [FLEX],  [fast lexical analyzer generator])

URBI_PROG_CXX
# The kernel is far from being ready for this warning.
TC_CXX_WARNINGS([-Wno-effc++])

# We don't need static libraries, speed the compilation up.
AC_ENABLE_SHARED([no])
AC_PROG_LIBTOOL


## --------- ##
## Network.  ##
## --------- ##
URBI_OPENR
AM_CONDITIONAL([NETWORK_OPENR],  [$openr])


## ------------- ##
## liburbi C++.  ##
## ------------- ##
AC_CONFIG_SUBDIRS([liburbi-c++])

## -------- ##
## Output.  ##
## -------- ##

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
