// The main point of this file is to save cycles: version.hh changes
// frequently, so include it in a small file instead of userver.cc
// which is demanding.

#include <kernel/userver.hh>

#include <libport/package-info.hh>

#include <sdk/config.h>
#include <version.hh>
#include <kernel/ubanner.hh>

// Standard header used by the server. Divided into "before" and
// "after" the custom header defined by the real server.

const char* HEADER_BEFORE_CUSTOM[] =
  {
    "*** **********************************************************\n",
    "*** URBI Kernel version " PACKAGE_VERSION_REV "\n",
    "*** Copyright (C) " PACKAGE_COPYRIGHT_YEARS " " PACKAGE_COPYRIGHT_HOLDER "\n",
    "***\n",
    0
  };

const char* HEADER_AFTER_CUSTOM[] =
  {
    "***\n",
    "*** URBI comes with ABSOLUTELY NO WARRANTY;\n",
    "*** This software can be used under certain conditions;\n",
    "*** see LICENSE file for details.\n",
    "***\n",
    "*** See http://www.urbiforge.com for news and updates.\n",
    "*** **********************************************************\n",
    0
  };

const char* uconsole_banner[] =
{
  "***      URBI Kernel Console " PACKAGE_VERSION_REV "\n",
  "***      "
  "Copyright (C) " PACKAGE_COPYRIGHT_YEARS " " PACKAGE_COPYRIGHT_HOLDER "\n",
  ""
};

const libport::PackageInfo&
UServer::package_info ()
{
  LIBPORT_PACKAGE_INFO_STATIC_VAR(pi);
  return pi;
}

std::ostream&
userver_package_info_dump (std::ostream& o)
{
  return o << UServer::package_info();
}
