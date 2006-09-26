m4_pattern_forbid([^URBI_])dnl

AC_PREREQ([2.60])


# URBI_DIRS(DEFAULT-URBI-ENV)
# ---------------------------
# Define the directory variables we are about to use.
AC_DEFUN([URBI_DIRS],
[# We need to know the host (the type of architecture it will run on),
# the environment (the runtime, the event loop: posix, webots, aibo).

# URBI_HOST
AC_ARG_ENABLE([host],
	      [AC_HELP_STRING([--enable-host=urbi-host],
			      [The machine this will run on [HOST]])])
AC_REQUIRE([AC_CANONICAL_HOST])
AC_MSG_CHECKING([for URBI host type])
case $enable_host:$host_alias in
  '':'') URBI_HOST=$host;;
  '':* ) URBI_HOST=$host_alias;;
   *:* ) URBI_HOST=$enable_host;;
esac
AC_MSG_RESULT([$URBI_HOST])
AC_SUBST([URBI_HOST])

# URBI_ENV
AC_ARG_ENABLE([env],
	      [AC_HELP_STRING([--enable-env=urbi-env],
			      [The environment this will run on:
			       aibo, webots. posix [posix]])])
AC_MSG_CHECKING([for URBI environment type])
case $enable_env in
			'') URBI_ENV=$1;;
  remote|posix|webots|aibo) URBI_ENV=$enable_env;;
		  *) AC_MSG_ERROR([invalid urbi env: $enable_env]);;
esac
AC_MSG_RESULT([$URBI_ENV])
AC_SUBST([URBI_ENV])

# Everything is installed in $prefix/gostai.
AC_SUBST([PACKAGE_BRAND], [gostai])
AC_SUBST([branddir], ['$(prefix)/$(PACKAGE_BRAND)'])

# Standard headers and libraries are installed in regular
# includedir and libdir under $prefix.

# /usr/local/gostai/core/$host
AC_SUBST([hostdir], ['$(brandir)/core/$(URBI_HOST)'])

# /usr/local/gostai/core/$host/$env.
# Could have been named sdklibdir too.
AC_SUBST([envdir], ['$(hostdir)/$(URBI_ENV)'])

# Where we install, and expect to find, headers.
AC_SUBST([sdkincludedir],  ['$(branddir)/include'])
CPPFLAGS="$CPPFLAGS -I$sdkincludedir"
LDFLAGS="$LDFLAGS -L$envdir"
])

## Local Variables:
## mode: autoconf
## End:
