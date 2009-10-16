/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// This file *must* remain with absolutely no run-time dependency.

#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <libport/windows.hh>
#include <libport/foreach.hh>

#include "urbi-root.hh"

// Quick hackish portability layer. We cannot use the one from libport as
// it is not header-only.
#ifdef WIN32
static const int RTLD_LAZY = 0;
static const int RTLD_GLOBAL = 0;
static const char* LD_LIBRARY_PATH_NAME = "LD_LIBRARY_PATH";


static HMODULE
dlopen(const char* name, int)
{
  return LoadLibrary(name);
}

static void*
dlsym(HMODULE module, const char* name)
{
  return GetProcAddress(module, name);
}

static const char*
dlerror(DWORD err = GetLastError())
{
  static char buf[1024];

  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                0, err, 0,
                (LPTSTR)buf, sizeof buf,
                0);

  return buf;
}

static strings_type
split(std::string lib)
{
  strings_type res;
  size_t pos;
  while ((pos = lib.find_first_of(':')) != lib.npos)
  {
    std::string s = lib.substr(0, pos);
    lib = lib.substr(pos + 1, lib.npos);
    // In case we split "c:\foo" into "c" and "\foo", glue them
    // together again.
    if (s[0] == '\\'
        && !res.empty()
        && res.back().length() == 1)
      res.back() += ':' + s;
    else
      res.push_back(s);
  }
  if (!lib.empty())
    res.push_back(lib);
  return res;
}
#else
typedef void* HMODULE;
# include <dlfcn.h>
#endif

/// Main. Load liburbi and call urbi_launch
int main(int argc, char* argv[])
{
  /* Unfortunately, shared object search path cannot be modified once
   * the program has started under linux (see dlopen(3)).
   * Plan A would be to load our dependencies manualy, which implies
   * knowing all of them and keeping this file up-to-date.
   * Plan B is simpler: setenv and exec
   */
  std::string urbi_root = get_urbi_root(argv[0]);
  std::string libdir = urbi_root + "/" + lib_rel_path;

  const char* c_ld_lib_path = getenv(LD_LIBRARY_PATH_NAME);
  std::string ld_lib_path = c_ld_lib_path ? c_ld_lib_path:"";

#ifndef WIN32
  // Only set URBI_ROOT if not allready present.
  setenv("URBI_ROOT", urbi_root.c_str(), 0);

  // Look if what we need is in (DY)LD_LIBRARY_PATH. Proceed if it is.
  // Otherwise, add it and exec ourselve to retry.
  if (ld_lib_path.find(libdir) == ld_lib_path.npos)
  {
    setenv(LD_LIBRARY_PATH_NAME, (ld_lib_path +":" + libdir).c_str(), 1);
    execv(argv[0], argv);
  }
#else
  _putenv(("URBI_ROOT=" +urbi_root).c_str());
  char* c_path = getenv("PATH");
  std::string path = c_path?c_path:"";
  strings_type ldpath_list = split(ld_lib_path);
  foreach(const std::string& s, ldpath_list)
    path += ";" + s;
  std::cerr <<  ("ENV PATH=" + path) << std::endl;
  _putenv( ("PATH=" + path).c_str());
#endif

  std::string liburbi_path = std::string() +  libdir + "/liburbi" + lib_ext;

  // First try using LIBRARY_PATH in the environment.
  HMODULE handle = dlopen((std::string("liburbi") + lib_ext).c_str(),
    RTLD_LAZY | RTLD_GLOBAL);

  // If it fails, force path based on URBI_ROOT.
  if (!handle)
    handle = dlopen(liburbi_path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if (!handle)
  {
    std::cerr << "Failed to open liburbi: " << liburbi_path << ':'
     << dlerror() << std::endl;
    std::cerr << getenv(LD_LIBRARY_PATH_NAME) << std::endl;
    return 1;
  }
  typedef int(*main_t)(int, char*[]);
  main_t urbi_launch = (main_t)dlsym(handle, "urbi_launch");
  if (!urbi_launch)
  {
    std::cerr << "Failed to locate urbi_launch symbol: "
     << dlerror() << std::endl;
    return 2;
  }
  return urbi_launch(argc, argv);
}
