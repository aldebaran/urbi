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

#define ECHO(S)                                 \
  std::cerr << S << std::endl

// Quick hackish portability layer. We cannot use the one from libport as
// it is not header-only.
#ifdef WIN32
#define RTLD_LAZY 0
#define RTLD_GLOBAL 0

static HMODULE
dlopen(const char* name, int)
{
  HMODULE res = LoadLibrary(name);
  if (res)
  {
    char buf[BUFSIZ];
    GetModuleFileName(res, buf, sizeof buf - 1);
    buf[sizeof buf - 1] = 0;
    ECHO("loaded: " << buf);
  }
  return res;
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

std::string
xgetenv(const char* var)
{
  const char* res = getenv(var);
  return res ? res : "";
}

void
xsetenv(const std::string& var, const std::string& val, int force)
{
#ifdef WIN32
  (void) force;
  _putenv(strdup((var + "=" + val).c_str()));
#else
  setenv(var.c_str(), val.c_str(), force);
#endif
}

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

  std::string ld_lib_path = xgetenv(LD_LIBRARY_PATH_NAME);

#ifndef WIN32
  // Only set URBI_ROOT if not already present.
  xsetenv("URBI_ROOT", urbi_root, 0);

  // Look if what we need is in (DY)LD_LIBRARY_PATH. Proceed if it is.
  // Otherwise, add it and exec ourselve to retry.
  if (ld_lib_path.find(libdir) == ld_lib_path.npos)
  {
    xsetenv(LD_LIBRARY_PATH_NAME, ld_lib_path + ":" + libdir, 1);
    execv(argv[0], argv);
  }
#else
  xsetenv("URBI_ROOT", urbi_root, 1);
  std::string path = xgetenv("PATH");
  strings_type ldpath_list = split(ld_lib_path);
  foreach(const std::string& s, ldpath_list)
    path += ";" + s;
  ECHO("ENV PATH=" << path);
  xsetenv("PATH", path);
#endif

  std::string liburbi_path = std::string() + libdir + "/liburbi" + lib_ext;

  // First try using LIBRARY_PATH in the environment.
  HMODULE handle = dlopen((std::string("liburbi") + lib_ext).c_str(),
                          RTLD_LAZY | RTLD_GLOBAL);

  // If it fails, force path based on URBI_ROOT.
  if (!handle)
    handle = dlopen(liburbi_path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if (!handle)
  {
    ECHO("cannot open liburbi: " << liburbi_path << ':' << dlerror());
    ECHO(getenv(LD_LIBRARY_PATH_NAME));
    return 1;
  }
  typedef int(*main_t)(int, char*[]);
  main_t urbi_launch = (main_t)dlsym(handle, "urbi_launch");
  if (!urbi_launch)
  {
    ECHO("cannot locate urbi_launch symbol: " << dlerror());
    return 2;
  }
  return urbi_launch(argc, argv);
}
