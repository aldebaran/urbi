m4_pattern_forbid([^ACX_])
m4_pattern_forbid([^AX_])
m4_pattern_forbid([^URBI_])

AC_PREREQ([2.60])
AC_INIT([Urbi SDK], [UNDEFINED],
        [kernel-bugs@lists.gostai.com], [urbi-sdk])
URBI_PACKAGE_VERSION
# FIXME: Disable the generation of CPP symbols defining the current
# version, as it is unclear what it should be.
# URBI_KERNEL_VERSION
URBI_PACKAGE_COPYRIGHT([Gostai SAS], [2005-2009])
URBI_DISTCHECK_CONFIGURE_FLAGS

# Libtoolize needs to see this invocation...
AC_CONFIG_AUX_DIR([build-aux])
URBI_BUILD_AUX

# Automake.
AM_INIT_AUTOMAKE([1.10 foreign tar-ustar nostdinc subdir-objects
                  -Wall -Wno-portability])

## ------------ ##
## Parameters.  ##
## ------------ ##

URBI_PACKAGE_KIND([kernel])
URBI_DIRS([engine])

URBI_DOC
AC_CONFIG_FILES([doc/config.sty])

## ------------- ##
## Compilation.  ##
## ------------- ##


# Bison.
AC_CONFIG_FILES([build-aux/bison++], [chmod +x build-aux/bison++])
# If we have our own Bison, use it.
if test -d $srcdir/bison; then
  (
    test -d bison || mkdir bison
    abs_srcdir=$(cd "$srcdir" && pwd)
    cd bison
    # Unset everything that puts us in cross-compilation mode,
    # as Bison is a build tool.
    unset CPPFLAGS CC CFLAGS CXX CXXFLAGS LD AR host host_alias
    "$abs_srcdir"/bison/configure
  )
  AC_SUBST([BISON], [$(pwd)/bison/tests/bison])
else
  TC_PROG([bison], [>= 2.3a+], [BISON], [GNU parser generator])
fi

# Flex.  2.3.33 has a problem with REJECT and produces a scanner that
# does not compile.
AC_CONFIG_FILES([build-aux/flex++],  [chmod +x build-aux/flex++])
TC_PROG([flex],  [>= 2.5.35], [FLEX],  [fast lexical analyzer generator])

URBI_PROG_CXX
URBI_COMPILATION_MODE
TC_CXX_WARNINGS([[-Wcast-align],
		 [-Wcast-qual],
		 [-Wmissing-prototypes],
		 [-Wstrict-prototypes],
		 [-Wwrite-strings],
		 [-Wbad-function-cast],
		 [-Wmissing-declarations]])

# We compile libsched.
AM_PROG_AS

# Disable static libraries. Loading modules as shared libraries
# requires us to be one.
AC_ENABLE_STATIC([no])
URBI_LIBTOOL

URBI_ARG_ENABLE([enable-headers-install],
                [Install kernel headers], [yes|no], [no])
AM_CONDITIONAL([INSTALL_KERNEL_HEADERS],
               [test x$enable_headers_install = xyes])

URBI_ARG_ENABLE([enable-serialization],
                [Build with support for serialization], [yes|no], [yes])
AM_CONDITIONAL([ENABLE_SERIALIZATION],
               [test x$enable_serialization = xyes])
if test x$enable_serialization = xyes; then
  AC_DEFINE([ENABLE_SERIALIZATION], [1], [Define to 1 if serialization is enabled.])
fi

# Where we look for the installed files.
# Do not use $package here, kernel is a too generic name.
pkgdatadir=${datadir}/gostai
AC_SUBST([urbidir], [${pkgdatadir}/urbi])


## -------- ##
## Target.  ##
## -------- ##
# URBI_LIBPORT requires 1.34, but it may be too soon to update its
# request, since the rest of the dev team might not have Boost 1.35.
# So require it here.
BOOST_REQUIRE([1.37])
# URBI_LIBPORT calls the Boost macros first, so keep other Boost stuff
# below.
URBI_LIBPORT([sdk-remote/libport])
URBI_PTHREAD
URBI_SOCKETS

URBI_ARG_ENABLE([enable-programs],
                [Build extra programs], [yes|no], [no])
AM_CONDITIONAL([BUILD_PROGRAMS],
               [test x$enable_programs = xyes])
BOOST_TOKENIZER
BOOST_SYSTEM


## -------------- ##
## Sub packages.  ##
## -------------- ##

AC_CONFIG_SUBDIRS([sdk-remote])

## -------- ##
## Output.  ##
## -------- ##

URBI_TESTS
AC_SUBST([UOBJECT_PATH], ['$(abs_top_builddir)/sdk-remote/src/uobjects'])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_CONFIG_FILES([scripts/package],  [chmod +x scripts/package])
AC_CONFIG_FILES([scripts/urbi.bat], [chmod +x scripts/urbi.bat])
AC_CONFIG_HEADERS([src/kernel/config.h])
AC_CONFIG_FILES([share/urbi/platform.u])
AC_OUTPUT
