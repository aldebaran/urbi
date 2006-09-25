m4_pattern_forbid([^URBI_])dnl

AC_PREREQ([2.60])

# URBI_DIRS(TARGET)
# -----------------
# Define the directory variables we are about to use.
AC_DEFUN([URBI_DIRS],
[# pkgdatadir is usually related to the package name.  Here is
# is related to the brand.
AC_SUBST([PACKAGE_BRAND], [gostai])

AC_SUBST([TARGET], [$1])

AC_SUBST([branddir],    ['$(datadir)/$(PACKAGE_BRAND)'])
AC_SUBST([targetdir],   ['$(branddir)/$(TARGET)'])
AC_SUBST([includedir],  ['$(branddir)/include'])
AC_SUBST([libdir],      ['$(branddir)/lib'])
])

## Local Variables:
## mode: autoconf
## End:
