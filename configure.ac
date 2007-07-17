m4_pattern_forbid([^ACX_])dnl
m4_pattern_forbid([^AX_])dnl
m4_pattern_forbid([^URBI_])dnl

AC_PREREQ([2.60])
AC_INIT([kernel], [2.0])

# Libtoolize needs to see this invocation...
AC_CONFIG_AUX_DIR([build-aux])
URBI_BUILD_AUX

# Automake.
AM_INIT_AUTOMAKE([1.10 foreign tar-ustar -Wall dist-zip nostdinc])

## ------------ ##
## Parameters.  ##
## ------------ ##

URBI_PACKAGE_KIND([kernel])
URBI_DIRS([engine])

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
AC_CONFIG_FILES([Makefile
		   src/Makefile
		     src/ast/Makefile
		     src/object/Makefile
		     src/runner/Makefile])
AC_OUTPUT
