/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#ifndef URBI_EXPORT_HH
# define URBI_SDK_EXPORT_HH

# include <libport/detect-win32.h>

# ifdef WIN32
#  ifdef BUILDING_URBI_SDK
#   define URBI_SDK_API __declspec(dllexport)
#  else
#   define URBI_SDK_API __declspec(dllimport)
#  endif
# else
#  define URBI_SDK_API __attribute__((visibility("default")))
# endif

#endif // URBI_EXPORT_HH
