// The main point of this file is to save cycles: version.hh changes
// frequently, so include it in a small file instead of userver.cc
// which is demanding.

#include "config.h"
#include "version.hh"
#include "ubanner.hh"

// Standard header used by the server. Divided into "before" and
// "after" the custom header defined by the real server.

const char* HEADER_BEFORE_CUSTOM[] =
  {
    "*** **********************************************************\n",
    "*** URBI Language specif 1.0 - Copyright 2006-2007 Gostai SAS\n",
    "*** URBI Kernel version " PACKAGE_VERSION " rev." PACKAGE_REVISION "\n",
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
