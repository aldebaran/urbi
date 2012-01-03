/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_URBI_EXPORT_HH
# define OBJECT_URBI_EXPORT_HH

# include <libport/compiler.hh>

# ifdef BUILDING_URBI_MODULE
#  define URBI_MODULE_API ATTRIBUTE_DLLEXPORT
# else
#  define URBI_MODULE_API ATTRIBUTE_DLLIMPORT
# endif

#endif // !OBJECT_URBI_EXPORT_HH
