#include "kernel/userver.hh"

// The following file includes "config.h", unfortunately, it's going
// to catch libport/config.h because when one includes "dir/foo.h"
// which includes "bar.h", then it's "dir/bar.h" that takes precedence
// over the local "bar.h" :(
// 
// So we force the loading of src/config.h.  This is fragile, but I
// don't have time for something more intelligent now -- Akim.
#include "src/config.h"

#include "libport/package-info.hcc"

const libport::PackageInfo&
UServer::package_info ()
{
  static libport::PackageInfo pi;
  return pi;
}
