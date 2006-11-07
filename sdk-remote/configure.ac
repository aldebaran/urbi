m4_pattern_forbid([^ACX_])dnl
m4_pattern_forbid([^URBI_])dnl

AC_PREREQ([2.60])
AC_INIT([liburbi-c++], [1.0-b2], [bugs@gostai.com],
	[liburbi-c++])

# Auxiliary files.
AC_CONFIG_AUX_DIR([build-aux])
URBI_BUILD_AUX

# Automake.
AM_INIT_AUTOMAKE([1.9.6 -Wall tar-ustar dist-zip nostdinc])

# Urbi standard locations.
URBI_DIRS([remote])

URBI_SDK

# Doc generation.
URBI_DOC

## ------------- ##
## Compilation.  ##
## ------------- ##

URBI_LIBPORT

# For the time being, we are not interested about deprecated
# headers such as hash_map.h.  We should use ext/hash_map.
AC_PROG_CC
URBI_PROG_CXX
# Unfortunately -Weffc++ triggers warnings that cannot be avoided
# when using unions in classes.
TC_CXX_WARNINGS([-Wno-effc++])

# We don't need shared libraries, speed the compilation up.
AC_ENABLE_SHARED([no])
AC_PROG_LIBTOOL

# X11.
AC_PATH_XTRA
AM_CONDITIONAL([X11], [test x"$no_x" != xyes])

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
 SWIG_ENABLE_CXX()
 fi
fi

## -------- ##
## Output.  ##
## -------- ##

URBI_CONFIG_HEADERS_PREFIXED([src/config.h],
	  		     [libport/config.h], [LIBPORT])
AC_CONFIG_SUBDIRS([jpeg-6b])
AC_CONFIG_FILES([Makefile
	  libport/Makefile
	  libport/tests/Makefile
	  src/Makefile
	  src/examples/Makefile
	  src/liburbi/Makefile
	  src/liburbi/swig/Makefile
	  src/utils/Makefile
])
AC_OUTPUT
