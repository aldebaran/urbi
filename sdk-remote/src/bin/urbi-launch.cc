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

// Quick hackish portability layer. We cannot use the one from libport as
// it is not header-only.
#ifdef WIN32
  static const char* lib_rel_path = "bin";
  static const char* lib_ext = ".dll";
  static const char* LD_LIBRARY_PATH_NAME = "PATH";
  void* dlopen(const char* name, int)
  {
    return LoadLibrary(name);
  }
  void* dlsym(void* module, const char* name)
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

#else
  #include <dlfcn.h>
  static const char* lib_rel_path = "lib";
# ifdef __APPLE__
  static const char* LD_LIBRARY_PATH_NAME = "DYLD_LIBRARY_PATH";
  static const char* lib_ext = ".dylib";
# else
  static const char* LD_LIBRARY_PATH_NAME = "LD_LIBRARY_PATH";
  static const char* lib_ext = ".so";
# endif
#endif

/// Return the content of URBI_ROOT, or extract it from argv[0].
std::string get_urbi_root(const char* arg0)
{
  const char* uroot = getenv("URBI_ROOT");
  if (uroot)
    return uroot;
  int p = 0;
  for (p = strlen(arg0)-1; p>=0 && arg0[p] != '/' && arg0[p] != '\\'
                  ; --p)
       ;
  if (p<0)
    return ".";
  std::string bindir = std::string(arg0, arg0 + p);
  if (bindir.rfind("bin") == bindir.size() - 3)
    return bindir.substr(0,  bindir.size() - 3);
  else
    return bindir + "/..";
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

#ifndef WIN32
  // Only set URBI_ROOT if not allready present.
  setenv("URBI_ROOT", urbi_root.c_str(), 0);

  // Look if what we need is in (DY)LD_LIBRARY_PATH. Proceed if it is.
  // Otherwise, add it and exec ourselve to retry.
  const char* c_ld_lib_path = getenv(LD_LIBRARY_PATH_NAME);
  std::string ld_lib_path = c_ld_lib_path ? c_ld_lib_path:"";
  if (ld_lib_path.find(libdir) == ld_lib_path.npos)
  {
    setenv(LD_LIBRARY_PATH_NAME, (ld_lib_path +":" + libdir).c_str(), 1);
    execv(argv[0], argv);
  }
#else
  _putenv(("URBI_ROOT=" +urbi_root).c_str());
#endif

  std::string liburbi_path = std::string() +  libdir + "/liburbi" + lib_ext;

  // First try using LIBRARY_PATH in the environment.
  void* handle = dlopen((std::string("liburbi") + lib_ext).c_str(),
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
