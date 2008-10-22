#include <sdk/config.h>
#include <version.hh>
#include <libport/package-info.hh>
#include <urbi/uobject.hh>

namespace urbi
{
  const libport::PackageInfo&
  package_info ()
  {
    LIBPORT_PACKAGE_INFO_STATIC_VAR(pi);
    return pi;
  }

}
