# This file is part of Urbi SDK.
# Copyright (C) 2006-2010, Gostai S.A.S.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
# See the LICENSE file for more information.
# For comments, bug reports and feedback: http://www.urbiforge.com
#
m4_pattern_forbid([^ACX_])
m4_pattern_forbid([^AX_])
m4_pattern_forbid([^URBI_])

AC_PREREQ([2.60])
AC_INIT([Urbi SDK], [2.3],
        [kernel-bugs@lists.gostai.com], [urbi-sdk])
URBI_PACKAGE_VERSION
# FIXME: Disable the generation of CPP symbols defining the current
# version, as it is unclear what it should be.
# URBI_KERNEL_VERSION
URBI_PACKAGE_COPYRIGHT([Gostai S.A.S.], [2005-2010])
URBI_DISTCHECK_CONFIGURE_FLAGS

# Libtoolize needs to see this invocation...
AC_CONFIG_AUX_DIR([build-aux])
URBI_BUILD_AUX

# Automake.
AM_INIT_AUTOMAKE([tar-ustar foreign nostdinc silent-rules subdir-objects -Wall])
AM_SILENT_RULES([yes])

## ------------ ##
## Parameters.  ##
## ------------ ##

URBI_PACKAGE_KIND([kernel])

URBI_DOC
URBI_ENABLE_DOC_SECTIONS([bioloid, nao, nxt, p3dx, rmp, spykee, webots])
AC_CONFIG_FILES([doc/config.sty doc/Makefile])

## ------------- ##
## Compilation.  ##
## ------------- ##


# Bison.
URBI_CONFIG_SCRIPTS([build-aux/bin/bison++])
# If we have our own Bison, use it.
if test -d $srcdir/bison; then
  # Run in a subshell, since we change the environment.
  AC_MSG_NOTICE([==================== configuring bison])
  if
    (
      test -d bison || mkdir bison
      abs_srcdir=$(cd "$srcdir" && pwd)
      cd bison
      # Unset everything that puts us in cross-compilation mode,
      # as Bison is a build tool.
      unset AR CC CFLAGS CPPFLAGS CXX CXXFLAGS host host_alias LD
      "$abs_srcdir"/bison/configure -C
    )
  then
    AC_MSG_NOTICE([==================== bison configured])
  else
    AC_MSG_ERROR([failed to configure bison])
  fi
  AC_SUBST([BISON], [$(pwd)/bison/tests/bison])
else
  TC_PROG([bison], [>= 2.3a+], [BISON], [GNU parser generator])
fi

# Flex.  2.3.33 has a problem with REJECT and produces a scanner that
# does not compile.
URBI_CONFIG_SCRIPTS([build-aux/bin/flex++])
TC_PROG([flex],  [>= 2.5.35], [FLEX],  [fast lexical analyzer generator])

URBI_COMPILATION_MODE
URBI_PROG_CXX
URBI_DIRS([engine])
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
AC_DISABLE_STATIC
URBI_LIBTOOL
URBI_SHLIBEXT

## ---------------- ##
## Build variants.  ##
## ---------------- ##

# --enable-headers-install.
URBI_ARG_ENABLE([enable-headers-install],
                [Install kernel headers], [yes|no], [no])
AM_CONDITIONAL([INSTALL_KERNEL_HEADERS],
               [test x$enable_headers_install = xyes])

# --enable-programs.
URBI_ARG_ENABLE([enable-programs],
                [Build extra programs], [yes|no], [no])
AM_CONDITIONAL([BUILD_PROGRAMS],
               [test x$enable_programs = xyes])

URBI_ARG_ENABLE([enable-static-build],
                [Static build], [yes|no], [no])
AM_CONDITIONAL([STATIC_BUILD],
               [test x$enable_static_build = xyes])
if test x$enable_static_build = xyes; then
  AC_DEFINE([STATIC_BUILD], [1],
            [Define to 1 if this is a static build.])
fi

## ------------ ##
## Components.  ##
## ------------ ##

URBI_BOOST_REQUIRE
URBI_BOOST([DATE_TIME])
URBI_BOOST([REGEX])
URBI_BOOST([SYSTEM])
URBI_BOOST([TOKENIZER])
URBI_BOOST_LOCATION

# URBI_LIBPORT calls the Boost macros first, so keep other Boost stuff
# below.
URBI_LIBPORT([sdk-remote/libport])
URBI_PTHREAD
URBI_SOCKETS
URBI_LIB_SUFFIX

WITH_XENOMAI
# object/directory.
AC_CHECK_HEADERS([sys/inotify.h])


## --------- ##
## modules.  ##
## --------- ##

AC_SUBST([modules_srcdir], ['$(top_srcdir)/modules'])
m4_include([modules/modules.ac])
URBI_MODULAR_BUILD
URBI_MODULE_REPORT
AC_CONFIG_FILES([modules/Makefile])


## -------------- ##
## Sub packages.  ##
## -------------- ##

AC_CONFIG_SUBDIRS([sdk-remote])

## -------- ##
## Output.  ##
## -------- ##

AC_DEFINE_UNQUOTED([SRCDIR], ["$srcdir"],
  [Define to the path from build to source tree.])

AC_SUBST([kernel_srcdir], ['${abs_top_srcdir}'])
AC_SUBST([kernel_builddir], ['${abs_top_builddir}'])
AC_SUBST([sdk_remote_srcdir], ['${kernel_srcdir}/sdk-remote'])
AC_SUBST([sdk_remote_builddir], ['${kernel_builddir}/sdk-remote'])
URBI_TESTS

AC_CONFIG_FILES([Makefile src/Makefile])

URBI_CONFIG_SCRIPTS([scripts/bf-post-install],
                    [scripts/install-boost],
                    [scripts/package])

case $URBI_HOST_OS in
  (windows) URBI_CONFIG_SCRIPTS([urbi.bat]);;
esac

AC_CONFIG_HEADERS([src/kernel/config.h])
AC_CONFIG_FILES([share/urbi/package-info.u share/urbi/platform.u])
AC_OUTPUT
