/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <config.h>

#include <libport/cerrno>
#include <iostream>
#include <string>
#include <vector>

#include <libport/lexical-cast.hh>

#include <libport/containers.hh>
#include <libport/cstdlib>
#include <libport/cstring>
#include <libport/sysexits.hh>
#include <libport/unistd.h>

#include <urbi/urbi-root.hh>

using libport::streq;
using libport::strneq;

int
main(int argc, char* argv[])
{
  UrbiRoot urbi_root(argv[0]);

  std::vector<std::string> args_urbi_launch;
  std::vector<std::string> args_libuobject;
  args_urbi_launch << "--start";

  for (int i = 1; i < argc; ++i)
  {
    if (streq(argv[i], "--debug") || streq(argv[i], "-d"))
      args_urbi_launch << "--debug" << argv[++i];
    else if (strneq(argv[i], "--debug=", 8))
      args_urbi_launch << "--debug" << argv[i] + 8;
    else
      args_libuobject << argv[i];
  }

  // Prepare a C version of vector<string> to please urbi_launch.
  const size_t argc_bounce =
    args_urbi_launch.size() + args_libuobject.size() + 2;
  const char** argv_bounce = new const char*[argc_bounce + 1];

  {
    unsigned i = 0;
    argv_bounce[i++] = argv[0];//urbi_launch.c_str();
    for (unsigned j = 0; j < args_urbi_launch.size(); ++j)
      argv_bounce[i++] = args_urbi_launch[j].c_str();
    argv_bounce[i++] = "--";
    for (unsigned j = 0; j < args_libuobject.size(); ++j)
      argv_bounce[i++] = args_libuobject[j].c_str();
    argv_bounce[i] = 0;
    aver(i == argc_bounce);
  }

  return urbi_root.urbi_launch(argc_bounce, argv_bounce);
}
