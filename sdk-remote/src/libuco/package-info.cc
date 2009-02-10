#include <libport/package-info.hh>
#include <libport/version.hh>

#include <sdk/config.h>
#include <urbi/package-info.hh>

#include <libuco/version.hh>

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
