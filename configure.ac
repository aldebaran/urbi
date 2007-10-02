m4_pattern_forbid([^ACX_])dnl
m4_pattern_forbid([^URBI_])dnl

AC_PREREQ([2.60])
AC_INIT([kernel1], [1.0])

# Libtoolize needs to see this invocation...
AC_CONFIG_AUX_DIR([build-aux])
URBI_BUILD_AUX

# Automake.
AM_INIT_AUTOMAKE([1.9.6 foreign tar-ustar -Wall dist-zip nostdinc])

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
TC_CXX_WARNINGS([[-Wcast-align],
 		 [-Wcast-qual],
 		 [-Wmissing-prototypes],
 		 [-Wstrict-prototypes],
 		 [-Wwrite-strings],
 		 [-Wbad-function-cast],
 		 [-Wmissing-declarations]])

# We don't need shared libraries, speed the compilation up.
AC_ENABLE_SHARED([no])
AC_PROG_LIBTOOL


## -------- ##
## Target.  ##
## -------- ##
URBI_OPENR
URBI_LIBPORT

## -------- ##
## Output.  ##
## -------- ##

AC_CONFIG_HEADERS([src/config.h])
URBI_TESTS
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
