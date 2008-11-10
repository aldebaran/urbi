m4_pattern_forbid([^URBI_])dnl

AC_PREREQ([2.60])

# URBI_SDK
# --------
# Instantiate the files required by SDKs.  Instantiate param.mk.
# The various flags that are needed by the SDK should be passed
# here.  This is painful: it must be kept up to date in two different
# places: where you detect these flags, and here.  But I have nothing
# better for the moment.  Hopefully flags such as LOCKSYSTEM_LIBS will
# be handled by libtool some day.
AC_DEFUN([URBI_SDK],
[AC_SUBST([BINDIR],
	  [$(URBI_RESOLVE_DIR([$bindir]))])
AC_SUBST_FILE([UMAKE_COMMON])
UMAKE_COMMON=$srcdir/sdk/umake-common
AC_CONFIG_FILES([sdk/umake],      [chmod +x sdk/umake])
AC_CONFIG_FILES([sdk/uinstall],   [chmod +x sdk/uinstall])
AC_CONFIG_FILES([sdk/umake-link], [chmod +x sdk/umake-link])
AC_CONFIG_FILES([sdk/wrapper.sh], [chmod +x sdk/wrapper.sh])
AC_CONFIG_HEADERS([sdk/config.h])
AC_CONFIG_FILES([sdk/param.mk],
		[perl -w ${srcdir}/sdk/eval_makefile.pl])
AC_SUBST([SDK_CFLAGS])
AC_SUBST([SDK_CXXFLAGS])
AC_SUBST([SDK_LDFLAGS])
AC_SUBST([SDK_LIBS])
AC_REQUIRE([URBI_SHLIBEXT])
])

## Local Variables:
## mode: autoconf
## End:
