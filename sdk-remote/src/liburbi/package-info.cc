#include "libport/package-info.hcc"

namespace urbi
{
  const libport::PackageInfo&
  liburbi_package_info ()
  {
    static libport::PackageInfo pi;
    return pi;
  }
}
