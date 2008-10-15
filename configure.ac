m4_pattern_forbid([^ACX_])
m4_pattern_forbid([^AX_])
m4_pattern_forbid([^URBI_])

AC_PREREQ([2.60])
AC_INIT([Urbi Kernel], [2.0-beta], [kernel-bugs@lists.gostai.com], [kernel])
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

URBI_COMPILATION_MODE

# Bison.
AC_CONFIG_FILES([build-aux/bison++], [chmod +x build-aux/bison++])
# If we have our own Bison, use it.
if test -d $srcdir/bison; then
  test -d bison || mkdir bison
  abs_srcdir=$(cd "$srcdir" && pwd)
  cd bison
  "$abs_srcdir"/bison/configure CC= CXX= LD= AR= CPPFLAGS= CXXFLAGS= CFLAGS=
  cd ..
  AC_SUBST([BISON], [$(pwd)/bison/tests/bison])
else
  TC_PROG([bison], [>= 2.3a+], [BISON], [GNU parser generator])
fi

# Flex.
AC_CONFIG_FILES([build-aux/flex++],  [chmod +x build-aux/flex++])
TC_PROG([flex],  [>= 2.5.33], [FLEX],  [fast lexical analyzer generator])

URBI_PROG_CXX
TC_CXX_WARNINGS([[-Wcast-align],
		 [-Wcast-qual],
		 [-Wmissing-prototypes],
		 [-Wstrict-prototypes],
		 [-Wwrite-strings],
		 [-Wbad-function-cast],
		 [-Wmissing-declarations]])

AM_PROG_AS

# Disable static libraries. Loading modules as shared libraries
# requires us to be one.
AC_ENABLE_STATIC([no])
AC_PROG_LIBTOOL

URBI_ARG_ENABLE([enable-headers-install],
                [Install kernel headers], [yes|no], [no])
AM_CONDITIONAL([INSTALL_KERNEL_HEADERS],
               [ test x$enable_headers_install = xyes])
URBI_WITH_LTDL

# Where we look for the installed files.
# Do not use $package here, kernel is a too generic name.
pkgdatadir=${datadir}/gostai
AC_SUBST([urbidir], [${pkgdatadir}/urbi])

# Can't use AC_REQUIRE outside an AC_DEFUN.  This appears to be the
# sole use of URBI_RESOLVE_DIR_PREPARE.
URBI_RESOLVE_DIR_PREPARE
AC_DEFINE_UNQUOTED([URBI_PATH],
                   ["`URBI_RESOLVE_DIR([${datadir}/gostai])`"],
		   [Define to the path where library files are installed.])


## -------- ##
## Target.  ##
## -------- ##
# URBI_LIBPORT requires 1.34, but it may be too soon to update its
# request, since the rest of the dev team might not have Boost 1.35.
# So require it here.
BOOST_REQUIRE([1.35])
# done by liburbi URBI_LIBPORT
# URBI_LIBPORT calls the Boost macros first, so keep other Boost stuff below.
URBI_PTHREAD

BOOST_FILESYSTEM
BOOST_REGEX
BOOST_THREADS
BOOST_TOKENIZER


SDK_LDFLAGS="$SDK_LDFLAGS $BOOST_FILESYSTEM_LDFLAGS"
SDK_LIBS="$SDK_LIBS $BOOST_FILESYSTEM_LIBS"


## ------ ##
## sdk/.  ##
## ------ ##

AC_CONFIG_SUBDIRS([sdk-remote])

## -------- ##
## Output.  ##
## -------- ##

URBI_LIBTOOL

URBI_TESTS
AC_CONFIG_FILES([Makefile src/Makefile])
AC_CONFIG_FILES([scripts/package], [chmod +x scripts/package])
AC_CONFIG_HEADERS([src/kernel/config.h])
AC_OUTPUT
