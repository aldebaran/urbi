m4_pattern_forbid([^ACX_])dnl
m4_pattern_forbid([^URBI_])dnl

AC_PREREQ([2.60])

# URBI_SDK
# --------
AC_DEFUN([URBI_SDK],
[AC_CONFIG_FILES([sdk/umake], [chmod +x sdk/umake])
AC_CONFIG_FILES([sdk/param.mk])
])

## Local Variables:
## mode: autoconf
## End:
