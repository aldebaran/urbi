/*
 * Copyright (C) 2007-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/package-info.hh>
#include <libport/version.hh>

#include <kernel/config.h>
#include <urbi/package-info.hh>

#include <libuvalue/revision-stub.hh>

namespace urbi
{
  const libport::PackageInfo&
  package_info()
  {
    LIBPORT_PACKAGE_INFO_STATIC_VAR(pi);
    static bool first = true;
    if (first)
    {
      first = false;
      pi.dependency_add(libport::package_info());
    }
    return pi;
  }

}
