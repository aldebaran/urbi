/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/version-check.hh>
#include <libport/package-info.hh>
#include <urbi/package-info.hh>

GD_CATEGORY(Urbi.Version);

namespace urbi
{
  int
  check_sdk_version(const std::string& where,
                    const libport::PackageInfo& compiler)
  {
    GD_FINFO_TRACE("Checking Urbi version for: %s", where);
    const libport::PackageInfo& loader = urbi::package_info();
    GD_FINFO_TRACE("Compiler: %s", compiler.name_version_revision());
    GD_FINFO_TRACE("Loader  : %s", loader.name_version_revision());
    if (loader.get("revision") != compiler.get("revision"))
    {
      std::string msg =
        libport::format("%s%sCompiled with %s, but loaded in %s",
                        where, where.empty() ? "" : ": ",
                        compiler.name_version_revision(),
                        loader.name_version_revision());

      const char* var = "URBI_ACCEPT_BINARY_MISMATCH";
      if (libport::getenv(var))
        GD_WARN(msg);
      else
      {
        msg += libport::format(", define %s to ignore", var);
        GD_ERROR(msg);
        throw std::runtime_error(msg);
      }
    }
    return 0;
  }

  VersionChecker::VersionChecker(const std::string& where,
                                 const libport::PackageInfo& compiler)
  {
    GD_CATEGORY(Urbi.Version);
    GD_INFO_DUMP("VersionChecker instantiating...");
    check_sdk_version(where, compiler);
  }

}
