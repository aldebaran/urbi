m4_pattern_forbid([^ACX_])
m4_pattern_forbid([^AX_])
m4_pattern_forbid([^URBI_])

AC_PREREQ([2.60])
AC_INIT([Urbi Kernel], [2.0], [kernel-bugs@lists.gostai.com], [kernel])
URBI_KERNEL_VERSION
URBI_PACKAGE_COPYRIGHT([Gostai SAS], [2005-2008])

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
AC_CONFIG_FILES([build-aux/flex++],  [chmod +x build-aux/flex++])
TC_PROG([bison], [>= 2.3a+], [BISON], [GNU parser generator])
TC_PROG([flex],  [== 2.5.4], [FLEX],  [fast lexical analyzer generator])

# FIXME: -fno-strict-aliasing must be removed once we use a Bison with
# variant and do not need the PODcaster anymore.
URBI_PROG_CXX
TC_CXX_WARNINGS([[-Wcast-align],
		 [-Wcast-qual],
		 [-Wmissing-prototypes],
		 [-Wstrict-prototypes],
		 [-Wwrite-strings],
		 [-Wbad-function-cast],
		 [-Wmissing-declarations],
		 [-fno-strict-aliasing]])

AM_PROG_AS

# We don't need shared libraries, speed the compilation up.
AC_ENABLE_SHARED([no])
AC_PROG_LIBTOOL

# FIXME: Bind this with a more general debug/optimize compilation switch.
# Whether we want to precompile the symbols.
URBI_ARG_ENABLE([enable-precompiled-symbols],
                [Precompile the symbols], [yes|no], [no])
case $enable_precompiled_symbols in
  yes) AC_DEFINE([SYMBOLS_PRECOMPILED], 1,
                 [Define if Urbi symbols should be precompiled]);;
esac

# Where we look for the installed files.
pkgdatadir=${datadir}/$PACKAGE
AC_SUBST([urbidir], [${pkgdatadir}/urbi])
# Can't use AC_REQUIRE outside an AC_DEFUN.  This appears to be the
# sole use of URBI_RESOLVE_DIR_PREPARE.
URBI_RESOLVE_DIR_PREPARE
AC_DEFINE_UNQUOTED([URBI_PATH],
                   ["`URBI_RESOLVE_DIR([$pkgdatadir])`"],
                   [Define to the path where library files are installed.])


## -------- ##
## Target.  ##
## -------- ##
URBI_OPENR
# URBI_LIBPORT requires 1.34, but it may be too soon to update its
# request, since the rest of the dev team might not have Boost 1.35.
# So require it here.
BOOST_REQUIRE([1.35])
URBI_LIBPORT
# URBI_LIBPORT calls the Boost macros first, so keep other Boost stuff below.
BOOST_TOKENIZER


## ------ ##
## sdk/.  ##
## ------ ##

URBI_SDK

## -------- ##
## Output.  ##
## -------- ##

URBI_TESTS
AC_CONFIG_FILES([Makefile src/Makefile lib/libport/Makefile])
AC_OUTPUT
