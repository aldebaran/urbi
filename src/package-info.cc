#include "kernel/userver.hh"
#include "libport/package-info.hcc"

const libport::PackageInfo&
UServer::package_info () const
{
  static libport::PackageInfo pi;
  return pi;
}
