/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <config.h>

#include <cerrno>
#include <iostream>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <libport/containers.hh>
#include <libport/cstdlib>
#include <libport/cstring>
#include <libport/sysexits.hh>
#include <libport/unistd.h>

#include "../sdk-remote/src/bin/urbi-root.hh"

using libport::streq;
using libport::strneq;

static std::string
xgetenv(const char* var, const std::string& def)
{
  if (const char* res = getenv(var))
    return res;
  else
    return def;
}

int
main(int argc, char* argv[])
{
  const std::string urbi_launch =
    xgetenv("URBI_LAUNCH",
            get_urbi_root(argv[0]) + "/bin/urbi-launch");

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

  // Prepare a C version of vector<string> to please exec.
  const size_t args_n = args_urbi_launch.size() + args_libuobject.size() + 3;
  const char** args_exec = new const char*[args_n];

  {
    unsigned i = 0;
    args_exec[i++] = urbi_launch.c_str();
    for (unsigned j = 0; j < args_urbi_launch.size(); ++j)
      args_exec[i++] = args_urbi_launch[j].c_str();
    args_exec[i++] = "--";
    for (unsigned j = 0; j < args_libuobject.size(); ++j)
      args_exec[i++] = args_libuobject[j].c_str();
    args_exec[i++] = 0;
    assert(i <= args_n);
  }

  execv(urbi_launch.c_str(), const_cast<char *const*>(args_exec));
  std::cerr
    << argv[0] << ": cannot exec " << urbi_launch
    << ": " << strerror(errno) << std::endl;
  exit(EX_OSFILE);
}
