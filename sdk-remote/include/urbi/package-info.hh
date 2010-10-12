/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_PACKAGE_INFO_HH
# define URBI_PACKAGE_INFO_HH

# include <libport/fwd.hh>
# include <urbi/export.hh>

namespace urbi
{

  /// Package information about liburbi and UObjects.
  URBI_SDK_API const libport::PackageInfo& package_info();

}

#endif // !URBI_PACKAGE_INFO_HH
