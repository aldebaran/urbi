m4_pattern_forbid([^ACX_])dnl
m4_pattern_forbid([^URBI_])dnl

AC_PREREQ([2.61])
AC_INIT([URBI SDK Remote], [1.6],
	[sdk-remote-bugs@gostai.com], [sdk-remote])
URBI_PACKAGE_COPYRIGHT([Gostai SAS], [2004-2008])

# Auxiliary files.
AC_CONFIG_AUX_DIR([build-aux])
URBI_BUILD_AUX

# Automake.
AM_INIT_AUTOMAKE([1.10 -Wall tar-ustar dist-zip nostdinc foreign])

# Urbi standard locations.
URBI_PACKAGE_KIND([sdk])
URBI_DIRS([remote])

AC_PROG_CC

URBI_PROG_CXX

AC_PROG_LIBTOOL


URBI_SDK

# Doc generation.
URBI_DOC

## ------------- ##
## Compilation.  ##
## ------------- ##

URBI_COMPILATION_MODE

# URBI_LIBPORT AC_REQUIREs URBI_PTHREAD, so run the latter first.
#URBI_PTHREAD
URBI_SOCKETS
URBI_LIBPORT
AC_CONFIG_FILES([lib/libport/Makefile])

BOOST_REQUIRE([1.35])

URBI_WITH_LTDL

# X11.
# AC_PATH_XTRA will find us X11 headers even if we are cross-compiling for windows
case $ac_cv_host in
(*pw32*|*win32*) no_x=yes;;
(*)  AC_PATH_XTRA ;;
esac

AM_CONDITIONAL([X11], [test x"$no_x" != xyes])
# FIXME: -lXext should be checked too.
if test x"$no_x" != xyes; then
  AC_SUBST([X11_LDADD],
	   ['$(X_PRE_LIBS) $(X_LIBS) -lX11 -lXext $(X_EXTRA_LIBS)'])
fi


# Examples.
AC_ARG_ENABLE([examples],
	      [AC_HELP_STRING([--disable-examples],
			      [do no compile the examples])],
	      [], [enable_examples=yes])
AC_MSG_CHECKING([whether compiling the examples])
AC_MSG_RESULT([$enable_examples])
AM_CONDITIONAL([EXAMPLES], [test x"$enable_examples" = xyes])

# Soundcard.
AC_CHECK_HEADERS([sys/soundcard.h], [soundcard=true], [soundcard=false])
AM_CONDITIONAL([SOUNDCARD], [$soundcard])



## --------- ##
## Bindings  ##
## --------- ##
AC_ARG_ENABLE([bindings],
	      [AC_HELP_STRING([--enable-bindings],
			      [enable some bindings])],
	      [], [enable_bindings=no])
AC_MSG_CHECKING([the bindings to activate])
save_IFS=$IFS
IFS=,
for i in $enable_bindings
do
  IFS=$save_IFS
  case $i in
    yes ) enable_bindings="csharp,java,lua,perl,php,python,ruby";;
    no | csharp | java | perl | php | python | ruby ) ;;
    *) AC_MSG_ERROR([invalid --enable-bindings argument: $i]);;
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
 AC_ENABLE_SHARED([yes])
 AC_PROG_SWIG([1.3.24])
 if (eval "$SWIG -version") >/dev/null 2>&1; then :; else
   AC_MSG_ERROR([SWIG 1.3.24 is required to build bindings.])
   SWIG_ENABLE_CXX
 fi
fi

URBI_LIBTOOL

AC_DEFINE_UNQUOTED([URBI_HOST], ["$host"], [Define as the host name.])
AC_DEFINE_UNQUOTED([URBI_PREFIX], ["$prefix"], [Define as the install prefix.])
AC_DEFINE_UNQUOTED([URBI_SHREXT], ["$shrext_cmds"],
                   [Define as the shared library extension])


## --------- ##
## libjpeg.  ##
## --------- ##

# Whether we should build (and test) the libjpeg executables.
# Reverses the default behavior in libjpeg, that's why we don't simply
# rely on configure's argument passing.
URBI_ARG_ENABLE([enable-programs],
		[Build extra programs], [yes|no], [no])
case $enable_programs in
  yes) # No need to pass --enable-programs, because (i) it's the
       # default for libjpeg, and (ii) if --enable-programs is
       # passed to this configure, it gives it to its subconfigures,
       # so we would just duplicate the request.
       ;;
  *)   ac_configure_args="$ac_configure_args --disable-programs";;
esac

AC_CONFIG_SUBDIRS([jpeg-6b])


## -------- ##
## Output.  ##
## -------- ##

AC_CONFIG_FILES([Makefile
                src/Makefile
	        src/swig/Makefile
                src/tests/Makefile
])
AC_OUTPUT
