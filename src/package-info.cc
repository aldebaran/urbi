#include "config.h"
#include "package-info.hh"
#include "version.hh"

PackageInfo::PackageInfo ()
  :
  bug_report   (PACKAGE_BUGREPORT),
  date         (PACKAGE_DATE),
  id           (PACKAGE_ID),
  name         (PACKAGE_NAME),
  revision     (PACKAGE_REVISION),
  string       (PACKAGE_STRING),
  tarname      (PACKAGE_TARNAME),
  version      (PACKAGE_VERSION),
  version_rev  (PACKAGE_VERSION_REV)
{}

const PackageInfo package_info;
