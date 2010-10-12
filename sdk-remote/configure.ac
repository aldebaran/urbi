# This file is part of Urbi SDK.
# Copyright (C) 2005-2010, Gostai S.A.S.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
# See the LICENSE file for more information.
# For comments, bug reports and feedback: http://www.urbiforge.com
#
m4_pattern_forbid([^ACX_])dnl
m4_pattern_forbid([^URBI_])dnl

AC_PREREQ([2.61])


## --------- ##
## AC_INIT.  ##
## --------- ##

AC_INIT([Urbi SDK Remote], [UNDEFINED],
        [sdk-remote-bugs@gostai.com], [sdk-remote])
URBI_PACKAGE_VERSION
URBI_PACKAGE_COPYRIGHT([Gostai S.A.S.], [2004-2010])
URBI_DISTCHECK_CONFIGURE_FLAGS

# Auxiliary files.
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([build-aux])
URBI_BUILD_AUX

# Automake.
AM_INIT_AUTOMAKE([1.11 -Wall tar-ustar dist-zip foreign nostdinc silent-rules])
AM_SILENT_RULES([yes])

# Urbi standard locations.
URBI_PACKAGE_KIND([sdk])

URBI_COMPILATION_MODE

AC_PROG_CC
URBI_PROG_CXX
URBI_DIRS([remote])

URBI_LIBTOOL
URBI_SDK

# Doc generation.
URBI_DOC
# Override "doc/sdk-remote" so that we still appears as a part
# of "urbi-sdk"'s documentation.
AC_SUBST([docdir], [${datarootdir}/doc/urbi-sdk])
AC_CONFIG_FILES([doc/sdk-remote.dox])

## ------------- ##
## Compilation.  ##
## ------------- ##

URBI_BOOST_REQUIRE

# URBI_LIBPORT AC_REQUIREs URBI_PTHREAD, so run the latter first.
URBI_SOCKETS
URBI_LIBPORT([libport])
URBI_BOOST([DATE_TIME])
URBI_BOOST([THREADS])

#ortp
PKG_CHECK_MODULES([ORTP], [ortp], [HAVE_ORTP=true], [HAVE_ORTP=false])
AM_CONDITIONAL([HAVE_ORTP], [$HAVE_ORTP])

# X11.
# AC_PATH_XTRA finds X11 headers even if cross-compiling to windows.
case $ac_cv_host in
  (*pw32*|*win32*|*mingw*) no_x=yes;;
  (*)  AC_PATH_XTRA ;;
esac

AM_CONDITIONAL([X11], [test x"$no_x" != xyes])
# FIXME: -lXext should be checked too.
if test x"$no_x" != xyes; then
  AC_SUBST([X11_LDADD],
           ['$(X_PRE_LIBS) $(X_LIBS) -lX11 -lXext $(X_EXTRA_LIBS)'])
fi
# We use -isystem for Boost if possible.  But then, if X11 is
# installed in the same place as another copy of Boost, because system
# directories are processed last, the Boost headers that will be used
# are those where X11 is installed (think of /opt/local/include on OS
# X for instance).  So look make X11 header's location isystem too.
#
# Note by the way that isystem is a tremendous help to fight Boost
# warnings.
AC_SUBST([X11_CPPFLAGS], [$X_CFLAGS])
URBI_ISYSTEM([X11_CPPFLAGS])


# Examples.
URBI_ARG_ENABLE([disable-examples],
                [do no compile the examples], [yes|no], [yes])
AC_MSG_CHECKING([whether compiling the examples])
AC_MSG_RESULT([$enable_examples])
AM_CONDITIONAL([EXAMPLES], [test x"$enable_examples" = xyes])

# --without-sdk-remote.
URBI_ARG_ENABLE([disable-sdk-remote],
                [do not compile the remote sdk], [yes|no], [yes])
AC_MSG_CHECKING([whether compiling the remote sdk])
AC_MSG_RESULT([$enable_sdk_remote])
AM_CONDITIONAL([SDK_REMOTE], [test x"$enable_sdk_remote" = xyes])

# Soundcard.
AC_CHECK_HEADERS([sys/soundcard.h], [soundcard=true], [soundcard=false])
AM_CONDITIONAL([SOUNDCARD], [$soundcard])



## --------- ##
## Bindings  ##
## --------- ##
URBI_ARGLIST_ENABLE([enable-bindings], [enable some bindings],
                    [yes|no|csharp|java|perl|php|python|ruby], [no])
AC_MSG_CHECKING([the bindings to activate])
save_IFS=$IFS
IFS=,
for i in $enable_bindings
do
  IFS=$save_IFS
  case $i in
    (yes) enable_bindings="csharp,java,lua,perl,php,python,ruby";;
    (no | csharp | java | perl | php | python | ruby) ;;
    (*) AC_MSG_ERROR([invalid --enable-bindings argument: $i]);;
  esac
done
AC_MSG_RESULT([$enable_bindings])

AM_CONDITIONAL([BINDINGS], [test x"$enable_bindings" != xno])
AM_CONDITIONAL([BINDING_CSHARP],
                [echo "$enable_bindings" | grep -w csharp >/dev/null])
AM_CONDITIONAL([BINDING_JAVA],
                [echo "$enable_bindings" | grep -w java >/dev/null])
AM_CONDITIONAL([BINDING_LUA],
                [echo "$enable_bindings" | grep -w lua >/dev/null])
AM_CONDITIONAL([BINDING_PERL],
                [echo "$enable_bindings" | grep -w perl >/dev/null])
AM_CONDITIONAL([BINDING_PHP],
                [echo "$enable_bindings" | grep -w php >/dev/null])
AM_CONDITIONAL([BINDING_PYTHON],
                [echo "$enable_bindings" | grep -w python >/dev/null])
AM_CONDITIONAL([BINDING_RUBY],
                [echo "$enable_bindings" | grep -w ruby >/dev/null])

if test x"$enable_bindings" != xno; then
 # With the bindings, we need shared libraries.
 AC_ENABLE_SHARED
 AC_PROG_SWIG([1.3.24])
 if (eval "$SWIG -version") >/dev/null 2>&1; then :; else
   AC_MSG_ERROR([SWIG 1.3.24 is required to build bindings.])
   SWIG_ENABLE_CXX
 fi
fi

WITH_XENOMAI

## --------- ##
## libjpeg.  ##
## --------- ##

# Whether we should build (and test) the libjpeg executables.
# Reverses the default behavior in libjpeg, that's why we don't simply
# rely on configure's argument passing.
URBI_ARG_ENABLE([enable-programs],
                [Build extra programs], [yes|no], [no])
case $enable_programs in
 (yes) # No need to pass --enable-programs, because (i) it's the
       # default for libjpeg, and (ii) if --enable-programs is
       # passed to this configure, it gives it to its subconfigures,
       # so we would just duplicate the request.
       ;;
  (*)  URBI_APPEND_CONFIGUREFLAGS([--disable-programs]);;
esac

# Rename libjpeg to libjpeg4urbi to avoid conflicts with system
# versions of the library.
URBI_APPEND_CONFIGUREFLAGS([LIBJPEG_NAME=jpeg4urbi])

URBI_ARG_ENABLE([enable-static-build],
                [Static build], [yes|no], [no])
AM_CONDITIONAL([STATIC_BUILD],
               [test x$enable_static_build = xyes])
if test x$enable_static_build = xyes; then
  AC_DEFINE([STATIC_BUILD], [1],
            [Define to 1 if this is a static build.])
fi


# Let libjpeg use the same libdir as us.
URBI_APPEND_CONFIGUREFLAGS(["libdir='${libdir}'"])
AC_CONFIG_SUBDIRS([jpeg])

# If we use a specific runtime for MSVC, tell it to libjpeg, which
# does not use the compilation-mode macros.
if test -n "$urbi_msvc_runtime"; then
  URBI_APPEND_CONFIGUREFLAGS([--enable-cflags=$urbi_msvc_runtime_compilerflags])
  URBI_APPEND_CONFIGUREFLAGS([--enable-ldflags=$urbi_msvc_runtime_ldflags])
fi

## -------- ##
## Output.  ##
## -------- ##
URBI_LIB_SUFFIX

URBI_APPEND_CONFIGUREFLAGS([LIBSFX=$LIBSFX])
AC_CONFIG_SUBDIRS([libport])

# For tests/.
AC_SUBST([kernel_srcdir], ['${abs_top_srcdir}/..'])
AC_SUBST([kernel_builddir], ['${abs_top_builddir}/..'])
AC_SUBST([sdk_remote_srcdir], ['${abs_top_srcdir}'])
AC_SUBST([sdk_remote_builddir], ['${abs_top_builddir}'])


AC_CONFIG_FILES([
  Makefile
    src/Makefile
      src/swig/Makefile
      src/tests/Makefile
])

# URBI_CONFIG_WRAPPER(COMPONENT-NAME)
# -----------------------------------
# Create a testing wrapper for the COMPONENT-NAME.
m4_define([URBI_CONFIG_WRAPPER],
[URBI_CONFIG_SCRIPTS([src/tests/bin/$1:src/tests/bin/wrapper.in])
])

# Prepare the umake wrappers.
URBI_CONFIG_WRAPPER([umake])
URBI_CONFIG_WRAPPER([umake-link])
URBI_CONFIG_WRAPPER([umake-shared])
URBI_CONFIG_SCRIPTS([src/tests/bin/urbi-launch$EXEEXT:src/tests/bin/urbi-launch.in])

URBI_CONFIG_SCRIPTS([src/tests/bin/urbi-send])

AC_OUTPUT
