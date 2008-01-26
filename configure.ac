m4_pattern_forbid([^ACX_])
m4_pattern_forbid([^AX_])
m4_pattern_forbid([^URBI_])

AC_PREREQ([2.60])
AC_INIT([Urbi Kernel], [2.0], [kernel-bugs@lists.gostai.com], [kernel])
URBI_KERNEL_VERSION
URBI_PACKAGE_COPYRIGHT([Gostai SAS], [2005-2007])

# Libtoolize needs to see this invocation...
AC_CONFIG_AUX_DIR([build-aux])
URBI_BUILD_AUX

# Automake.
AM_INIT_AUTOMAKE([1.10 foreign tar-ustar -Wall nostdinc])

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
TC_PROG([bison], [>= 2.3a+], [BISON], [GNU parser generator])
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
dnl URBI_LIBPORT calls the Boost macros first, so keep other Boost stuff below.
BOOST_TOKENIZER


## ------ ##
## sdk/.  ##
## ------ ##

URBI_SDK

## -------- ##
## Output.  ##
## -------- ##

URBI_TESTS
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
