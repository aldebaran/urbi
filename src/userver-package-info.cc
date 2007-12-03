#include "kernel/userver.hh"

#include "src/config.h"
#include "version.hh"
#include "libport/package-info.hh"

const libport::PackageInfo&
UServer::package_info ()
{
  LIBPORT_PACKAGE_INFO_STATIC_VAR(pi);
  return pi;
}
