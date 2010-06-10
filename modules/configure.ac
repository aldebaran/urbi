# This file is part of Urbi SDK.
# Copyright (C) 2006-2010, Gostai S.A.S.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
# See the LICENSE file for more information.
# For comments, bug reports and feedback: http://www.urbiforge.com

## --------------- ##
## Package setup.  ##
## --------------- ##

AC_INIT([MODULES], [1], [techteam@gostai.com], [Modules])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign -Wall])

## ---------------------- ##
## Tool/Compilers setup.  ##
## ---------------------- ##

URBI_COMPILATION_MODE

# Look for a C++ compiler.
URBI_PROG_CXX
URBI_PACKAGE_KIND([sdk])
URBI_DIRS
URBI_LIBTOOL
URBI_LIB_SUFFIX

# URBI_LIBPORT calls the Boost macros first, so keep other Boost stuff
# below.
URBI_LIBPORT([../sdk-remote/libport])

## ------------------ ##
## Handling modules.  ##
## ------------------ ##

m4_include([modules.ac])

URBI_MODULAR_BUILD
URBI_MODULE_REPORT
AC_CONFIG_FILES([Makefile urbi/modules.u])

AC_OUTPUT
