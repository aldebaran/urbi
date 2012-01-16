# This file is part of Urbi SDK.
# Copyright (C) 2006-2012, Gostai S.A.S.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
# See the LICENSE file for more information.
# For comments, bug reports and feedback: http://www.urbiforge.org
#
m4_pattern_forbid([^(ACX|AX|TC|URBI)_])dnl

AC_PREREQ([2.60])
AC_INIT([Urbi], [2.7.5],
        [kernel-bugs@lists.gostai.com], [urbi-sdk])
URBI_PACKAGE_VERSION
URBI_PACKAGE_COPYRIGHT([Gostai S.A.S.], [2004-2012])
URBI_DISTCHECK_CONFIGURE_FLAGS

# Libtoolize needs to see this invocation...
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([build-aux])
URBI_BUILD_AUX

# Automake.
AM_INIT_AUTOMAKE([foreign nostdinc silent-rules subdir-objects tar-ustar -Wall])
AM_SILENT_RULES([yes])

## ------------ ##
## Parameters.  ##
## ------------ ##

URBI_PACKAGE_KIND([kernel])
AC_SUBST([remotedir], ['${brandlibdir}/remote'])
AC_SUBST([enginedir], ['${brandlibdir}/engine'])
URBI_DOC
URBI_ENABLE_DOC_SECTIONS([bioloid, jazz, nao, nxt, p3dx, rmp, spykee, webots])
AC_CONFIG_FILES([doc/config.sty doc/Makefile doc/README.txt doc/COMPONENTS.txt])
COMPONENTS_FILE=doc/COMPONENTS.txt
AC_SUBST_FILE([COMPONENTS_FILE])
AM_PATH_LISPDIR

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
      unset AR CC CFLAGS CPPFLAGS CXX CXXFLAGS LDFLAGS host host_alias LD
      "$abs_srcdir"/bison/configure -C
    )
  then
    AC_MSG_NOTICE([==================== bison configured])
  else
    AC_MSG_ERROR([failed to configure bison])
  fi
  AC_SUBST([BISON], [$(pwd)/bison/tests/bison])
  AC_SUBST([BISON_VERSION],
           [$(cd "$srcdir/bison" &&
              build-aux/git-version-gen .tarball-version)])
else
  TC_PROG([bison], [>= 2.3a+], [BISON], [GNU parser generator])
fi

# Flex.  2.3.33 has a problem with REJECT and produces a scanner that
# does not compile.
URBI_CONFIG_SCRIPTS([build-aux/bin/flex++])
TC_PROG([flex],  [>= 2.5.35], [FLEX],  [fast lexical analyzer generator])

URBI_COMPILATION_MODE
URBI_PROG_CXX
AC_LANG([C++])
URBI_DIRS([engine])
TC_CXX_WARNINGS([[-Wcast-align],
		 [-Wcast-qual],
		 [-Wmissing-prototypes],
		 [-Wstrict-prototypes],
		 [-Wwrite-strings],
		 [-Wbad-function-cast],
		 [-Wmissing-declarations]])

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
URBI_BOOST([THREADS])
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

# ortp
URBI_ARG_ENABLE([disable-rtp],
                [disable support for RTP protocol], [yes|no], [yes])
PKG_CHECK_MODULES([ORTP], [ortp], [HAVE_ORTP=true], [HAVE_ORTP=false])
case $enable_rtp:$HAVE_ORTP in
  (yes:false)
    AC_MSG_ERROR([cannot find oRTP, although required.  Pass --disable-rtp to proceed])
    ;;
  (*:true)
    URBI_PKG_CONFIG_VERSION([ortp])
    ;;
esac
AC_SUBST([HAVE_ORTP])
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
URBI_ARGLIST_ENABLE([disable-bindings],
                    [disable the SWIG bindings to other languages],
                    [yes|no|java], [java])
AC_MSG_CHECKING([the bindings to activate])
save_IFS=$IFS
IFS=,
for i in $enable_bindings
do
  IFS=$save_IFS
  case $i in
    (yes) enable_bindings="java";;
    (no | java) ;;
    (*) AC_MSG_ERROR([invalid --enable-bindings argument: $i]);;
  esac
done
IFS=$save_IFS
AC_MSG_RESULT([$enable_bindings])

AM_CONDITIONAL([BINDINGS], [test x"$enable_bindings" != xno])
AM_CONDITIONAL([BINDING_JAVA],
               [echo "$enable_bindings" | grep -w java >/dev/null])

if test x"$enable_bindings" != xno; then
 # With the bindings, we need shared libraries.
 AC_ENABLE_SHARED
 AX_PKG_SWIG([1.3.36], [],
             [AC_MSG_ERROR([SWIG 1.3.36 is required to build bindings.])])
 AC_SUBST([SWIG_VERSION], [$swig_version])

 ## -------------------------- ##
 ## liburbi java configuration ##
 ## -------------------------- ##
 m4_include([sdk-remote/src/swig/java/liburbijava.ac])

 ac_configure_args=" $ac_configure_args --disable-programs"
fi

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


# Let libjpeg use the same libdir as us.
URBI_APPEND_CONFIGUREFLAGS(["libdir='${libdir}'"])
AC_CONFIG_SUBDIRS([sdk-remote/jpeg])

# If we use a specific runtime for MSVC, tell it to libjpeg, which
# does not use the compilation-mode macros.
if test -n "$urbi_msvc_runtime"; then
  URBI_APPEND_CONFIGUREFLAGS([--with-cflags=$urbi_msvc_runtime_compilerflags])
  URBI_APPEND_CONFIGUREFLAGS([--with-ldflags=$urbi_msvc_runtime_ldflags])
fi

## ----------------- ##
## Late SDK Remote.  ##
## ----------------- ##

AC_CONFIG_FILES([sdk-remote/doc/sdk-remote.dox])
AC_CONFIG_FILES([sdk-remote/doc/sdk-remote-java.dox])

# Instantiate the files required by SDKs.  Instantiate param.mk.
# The various flags that are needed by the SDK should be passed
# here.  This is painful: it must be kept up to date in two different
# places: where you detect these flags, and here.  But I have nothing
# better for the moment.  Hopefully flags such as LOCKSYSTEM_LIBS will
# be handled by libtool some day.
AC_SUBST([BINDIR],
         [$(URBI_RESOLVE_DIR([$bindir]))])
AC_SUBST_FILE([UMAKE_COMMON])
UMAKE_COMMON=$srcdir/sdk-remote/sdk/umake-common
URBI_CONFIG_SCRIPTS([sdk-remote/sdk/umake],
                    [sdk-remote/sdk/umake-link],
                    [sdk-remote/sdk/umake-java],
                    [sdk-remote/sdk/urbi-launch-java],
                    [sdk-remote/sdk/wrapper.sh])
AC_CONFIG_FILES([sdk-remote/sdk/param.mk])
AC_CONFIG_FILES([sdk-remote/sdk/java/param.mk])
AC_SUBST([SDK_CFLAGS])
AC_SUBST([SDK_CXXFLAGS])
AC_SUBST([SDK_LDFLAGS])
AC_SUBST([SDK_LIBS])

URBI_APPLE

## --------- ##
## modules.  ##
## --------- ##

AC_SUBST([modules_srcdir], ['$(top_srcdir)/modules'])
m4_include([modules/modules.ac])
URBI_MODULAR_BUILD
URBI_MODULE_REPORT
AC_CONFIG_FILES([modules/Makefile])


## -------- ##
## Output.  ##
## -------- ##

AC_SUBST([kernel_srcdir], ['${abs_top_srcdir}'])
AC_SUBST([kernel_builddir], ['${abs_top_builddir}'])
AC_SUBST([sdk_remote_srcdir], ['${kernel_srcdir}/sdk-remote'])
AC_SUBST([sdk_remote_builddir], ['${kernel_builddir}/sdk-remote'])
URBI_TESTS

AC_CONFIG_FILES([Makefile src/Makefile])

URBI_CONFIG_SCRIPTS([scripts/bf-post-install],
                    [scripts/common.sh],
                    [scripts/install-boost],
                    [scripts/install-java])

case $URBI_HOST_OS in
  (windows) URBI_CONFIG_SCRIPTS([urbi.bat]);;
esac

AC_CONFIG_HEADERS([src/kernel/config.h])
AC_CONFIG_FILES([
  share/pkgconfig/urbi.pc
  share/urbi/package-info.u
  share/urbi/platform$LIBSFX.u:share/urbi/platform.u.in],
  [], [LIBSFX=$LIBSFX])
URBI_APPEND_CONFIGUREFLAGS([LIBSFX=$LIBSFX])
AC_CONFIG_SUBDIRS([sdk-remote/libport])

AC_CONFIG_FILES([
  sdk-remote/Makefile
    sdk-remote/src/Makefile
      sdk-remote/src/swig/java/Makefile
      sdk-remote/src/tests/Makefile
])


# URBI_CONFIG_WRAPPER(COMPONENT-NAME, TEMPLATE)
# ---------------------------------------------
# Create an SDK Remote testing wrapper for the COMPONENT-NAME
# from TEMPLATE.
m4_define([URBI_CONFIG_WRAPPER],
[URBI_CONFIG_SCRIPTS(
  [sdk-remote/src/tests/bin/$1:sdk-remote/src/tests/bin/$2])])

# Prepare the umake wrappers.
URBI_CONFIG_WRAPPER([umake-java],         [wrapper.in])
URBI_CONFIG_WRAPPER([umake-link],         [wrapper.in])
URBI_CONFIG_WRAPPER([umake-shared],       [wrapper.in])
URBI_CONFIG_WRAPPER([umake],              [wrapper.in])
URBI_CONFIG_WRAPPER([urbi-launch$EXEEXT], [urbi-launch.in])
URBI_CONFIG_WRAPPER([urbi-launch-java],   [urbi-launch.in])
URBI_CONFIG_WRAPPER([urbi-send],          [urbi-send.in])


# Show what components are used.
AC_CONFIG_COMMANDS([report],
[
  AC_MSG_NOTICE([=========================])
  AC_MSG_NOTICE([components used:])
  while read i; do
    AC_MSG_NOTICE([ $i])
  done <doc/COMPONENTS.txt
  AC_MSG_NOTICE([=========================])
])

AC_OUTPUT
