/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/sys/stat.h>

#include <libport/cassert>
#include <libport/cstring>
#include <libport/cstdlib>
#include <libport/cstdio>
#include <iostream>

#include <libport/config.h>
#include <libport/foreach.hh>
#include <libport/sysexits.hh>

#include <urbi/urbi-root.hh>
#include <libport/config.h>

#if defined WIN32
# define APPLE_LINUX_WINDOWS(Apple, Linux, Windows) Windows
#elif defined __APPLE__
# define APPLE_LINUX_WINDOWS(Apple, Linux, Windows) Apple
#else
# define APPLE_LINUX_WINDOWS(Apple, Linux, Windows) Linux
#endif


/*--------.
| Helpers |
`--------*/

static std::string
mygetenv(const std::string& var, const std::string& val = "")
{
  const char* res = getenv(var.c_str());
  return res ? std::string(res) : val;
}

// static void
// xsetenv(const std::string& var, const std::string& val, int force)
// {
// #ifdef WIN32
//   if (force || mygetenv(var).empty())
//     _putenv(strdup((var + "=" + val).c_str()));
// #else
//   setenv(var.c_str(), val.c_str(), force);
// #endif
// }

/*------------.
| Reporting.  |
`------------*/

static bool
debug()
{
  static bool res = mygetenv("GD_LEVEL") == "DUMP";
  return res;
}

# define URBI_ROOT_ECHO(S)                      \
  std::cerr << S << std::endl                   \

# define URBI_ROOT_DEBUG(Self, S)                       \
  do {                                                  \
    if (debug())                                        \
      URBI_ROOT_ECHO(Self << ": " << S);                \
  } while (0)                                           \

# define URBI_ROOT_FATAL(Self, N, S)            \
  do {                                          \
    URBI_ROOT_ECHO(Self << ": " << S);          \
    exit(N);                                    \
  } while (0)

/*-----------------.
| File constants.  |
`-----------------*/

static const std::string libext =
                           APPLE_LINUX_WINDOWS(".dylib", ".so", ".dll");
static const std::string separator =
                           APPLE_LINUX_WINDOWS("/", "/", "\\");
static const std::string libdir =
                           APPLE_LINUX_WINDOWS("lib", "lib", "bin");

/// Join path components.
static
std::string
operator/(const std::string& lhs, const std::string& rhs)
{
  return (lhs.empty() ? rhs
          : rhs.empty() ? lhs
          : lhs + separator + rhs);
}

/*-------------------------------------.
| Crapy dynamic portability routines.  |
`-------------------------------------*/

#ifdef WIN32
#define RTLD_LAZY 0
#define RTLD_NOW 0
#define RTLD_GLOBAL 0

static RTLD_HANDLE
dlopen(const char* name, int)
{
  RTLD_HANDLE res = LoadLibrary(name);
  if (res)
  {
    char buf[BUFSIZ];
    GetModuleFileName(res, buf, sizeof buf - 1);
    buf[sizeof buf - 1] = 0;
  }
  return res;
}

static void*
dlsym(RTLD_HANDLE module, const char* name)
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
# include <dlfcn.h>
#endif

typedef std::vector<std::string> strings_type;
static strings_type
split(std::string lib)
{
  strings_type res;
  size_t pos;
  while ((pos = lib.find(':')) != lib.npos)
  {
    std::string s = lib.substr(0, pos);
    lib = lib.substr(pos + 1, lib.npos);
#ifdef WIN32
    // In case we split "c:\foo" into "c" and "\foo", glue them
    // together again.
    if (s[0] == '\\'
        && !res.empty()
        && res.back().length() == 1)
      res.back() += ':' + s;
    else
#endif
      res.push_back(s);
  }
  if (!lib.empty())
    res.push_back(lib);
  return res;
}

/// \a path does not include the extension.
static
RTLD_HANDLE
xdlopen(const std::string& program,
        const std::string& msg,
        std::string path,
        sysexit status = EX_FAIL,
        int flags = RTLD_LAZY | RTLD_GLOBAL)
{
  path += libext;
  URBI_ROOT_DEBUG(program, "loading library: " << path << " (" << msg << ")");
  if (RTLD_HANDLE res = dlopen(path.c_str(), flags))
    return res;
  else
    URBI_ROOT_FATAL(program, status,
                    "cannot open library: " << path << ": " << dlerror());
}

template <typename Res>
static
Res
xdlsym(const std::string& program,
       const char* modname, RTLD_HANDLE module,
       const char* name)
{
  URBI_ROOT_DEBUG(program, "loading symbol " << name << " from " << modname);
  // Reinterpret-cast fails with gcc3 arm.
  if (Res res = (Res)(dlsym(module, name)))
    return res;
  else
    URBI_ROOT_FATAL(program, 2,
                    "cannot locate " << name << " symbol: " << dlerror());
}


static
std::string
resolve_symlinks(const std::string& logname, const std::string& s)
{
#if defined WIN32
  return s;
#else
  char path[BUFSIZ];
  strncpy(path, s.c_str(), BUFSIZ);
  path[BUFSIZ - 1] = 0;
  while (readlink(path, path, BUFSIZ) != -1)
    URBI_ROOT_DEBUG(logname, "unrolling symbolic link: " << path);
  return path;
#endif
}

/*-----------.
| UrbiRoot.  |
`-----------*/

UrbiRoot::UrbiRoot(const std::string& program, bool static_build)
  : program_(program)
  , root_()
  , handle_libjpeg_(0)
  , handle_libport_(0)
  , handle_libsched_(0)
  , handle_liburbi_(0)
  , handle_libuobject_(0)
{
  // Find our directory.
  std::string uroot = mygetenv("URBI_ROOT");
  if (!uroot.empty())
  {
    URBI_ROOT_DEBUG(program_,
                    "URBI_ROOT is set, forcing root directory: " << root_);
    root_ = uroot;
  }
  else
  {
    URBI_ROOT_DEBUG(program_, "guessing Urbi root: invoked as: " << program_);
    // Handle the chained symlinks case.
    std::string argv0 = resolve_symlinks(program_, program);

#ifdef WIN32
    size_t pos = argv0.find_last_of("/\\");
#else
    size_t pos = argv0.rfind('/');
#endif
    if (pos == std::string::npos)
    {
      URBI_ROOT_DEBUG(program_,
                      "invoked from the path, looking for ourselves in PATH");
      strings_type path = split(mygetenv("PATH"));
      foreach (const std::string& dir, path)
      {
        struct stat stats;
        std::string file = dir / argv0;
        if (stat(file.c_str(), &stats) == 0)
        {
          URBI_ROOT_DEBUG(program_, "found: " << file);
          root_ = dir / "..";
          URBI_ROOT_DEBUG(program_, "root directory is: " << root_);
          break;
        }
        URBI_ROOT_DEBUG(program_, "not found: " << file);
      }
    }
    else
    {
      root_ = argv0.substr(0, pos) / "..";
      URBI_ROOT_DEBUG(program_,
                      "invoked with a path, setting root to parent directory: "
                      << root_);
    }
  }

  if (root_.empty())
    URBI_ROOT_FATAL(program_, 3,
                    "Unable to find Urbi SDK installation location. "
                    "Please set URBI_ROOT.");

  // const std::string urbi_path = root_ / "share" / "gostai";
  // xsetenv("URBI_PATH", mygetenv("URBI_PATH") + ":" + urbi_path, true);
  // URBI_ROOT_DEBUG("append to URBI_PATH: " << urbi_path);
  if (!static_build)
  {
    handle_libjpeg_      = library_load("jpeg");
    handle_libport_      = library_load("port");
    handle_libsched_     = library_load("sched");
    handle_libserialize_ = library_load("serialize");
    handle_liburbi_      = library_load("urbi");
  }
}

RTLD_HANDLE
UrbiRoot::library_load(const std::string& base)
{
  std::string envvar = "URBI_ROOT_LIB" + base;
  foreach (char& s, envvar)
    s = toupper(s);

  return
    xdlopen(program_,
            base,
            mygetenv(envvar,
                    root(libdir / "lib" + base + LIBPORT_LIBSFX)));
}

std::string
UrbiRoot::root(const std::string& path) const
{
  return root_ / path;
}

std::string
UrbiRoot::core_path(const std::string& path) const
{
  return root_ / "gostai" / "core" / LIBPORT_URBI_HOST / path;
}

std::string
UrbiRoot::uobjects_path(const std::string& path) const
{
  return core_path("uobjects" / path);
}

std::string
UrbiRoot::share_path(const std::string& path) const
{
  return root_ / "share" / "gostai" / path;
}

void
UrbiRoot::load_plugin()
{
  handle_libuobject_ =
    xdlopen(program_,
            "plugin UObject implementation",
            mygetenv("URBI_ROOT_LIBPLUGIN",
                     core_path() / "engine" / "libuobject" LIBPORT_LIBSFX),
            // This exit status is understood by the test suite.  It
            // helps it skipping SDK Remote tests that cannot run
            // without Urbi SDK.
            EX_OSFILE);
}

/// Location of Urbi remote libuobject
void
UrbiRoot::load_remote()
{
  handle_libuobject_ =
    xdlopen(program_,
            "remote UObject implementation",
            mygetenv("URBI_ROOT_LIBREMOTE",
                     core_path() / "remote" / "libuobject" LIBPORT_LIBSFX),
            EX_OSFILE);
}

void
UrbiRoot::load_custom(const std::string& path_)
{
  handle_libuobject_ =
    xdlopen(program_,
            "custom UObject implementation",
            path_ / "libuobject",
            EX_OSFILE);
}

typedef int(*urbi_launch_type)(int, const char*[], UrbiRoot&);
int
UrbiRoot::urbi_launch(int argc, const char** argv)
{
  urbi_launch_type f =
    xdlsym<urbi_launch_type>(program_,
                             "liburbi-launch", handle_liburbi_,
                             "urbi_launch");
  return f(argc, argv, *this);
}

int
UrbiRoot::urbi_launch(int argc, char** argv)
{
  return urbi_launch(argc, const_cast<const char**>(argv));
}

typedef int(*urbi_main_type)(const std::vector<std::string>& args,
                             UrbiRoot& root,
                             bool block, bool errors);
int
UrbiRoot::urbi_main(const std::vector<std::string>& args,
                    bool block, bool errors)
{
  urbi_main_type f =
    xdlsym<urbi_main_type>(program_,
                           "libuobject", handle_libuobject_,
                           "urbi_main_args");
  URBI_ROOT_DEBUG(program_, "command line: ");
  foreach (const std::string& arg, args)
    URBI_ROOT_DEBUG(program_, "  " << arg);

  return f(args, *this, block, errors);
}
