/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// The main point of this file is to save cycles: version.hh changes
// frequently, so include it in a small file instead of userver.cc
// which is demanding.

#include <libport/package-info.hh>
#include <urbi/package-info.hh>

#include <kernel/userver.hh>

#include <kernel/config.h>
#include <version.hh>
#include <kernel/ubanner.hh>

namespace kernel
{

  const libport::PackageInfo&
  UServer::package_info()
  {
    LIBPORT_PACKAGE_INFO_STATIC_VAR(pi);
    static bool tail = false;
    if (!tail++)
      pi.dependency_add(urbi::package_info());
    return pi;
  }

  std::ostream&
  userver_package_info_dump(std::ostream& o)
  {
    return o << UServer::package_info();
  }
}
