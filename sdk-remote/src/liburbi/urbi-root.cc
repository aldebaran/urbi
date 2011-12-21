/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/sys/stat.h>
#include <libport/unistd.h>

#include <libport/cassert>
#include <libport/cstring>
#include <libport/cstdlib>
#include <libport/cstdio>
#include <iostream>

#include <libport/config.h>
#include <libport/foreach.hh>
#include <libport/sysexits.hh>

#include <urbi/urbi-root.hh>
#ifdef STATIC_BUILD
# include <urbi/umain.hh>
#endif

#if defined WIN32
# define APPLE_LINUX_WINDOWS(Apple, Linux, Windows) Windows
#elif defined __APPLE__
# define APPLE_LINUX_WINDOWS(Apple, Linux, Windows) Apple
#else
# define APPLE_LINUX_WINDOWS(Apple, Linux, Windows) Linux
#endif

static std::string
mygetenv(const std::string& var, const std::string& val = "");

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

# define URBI_ROOT_DEBUG(Self, S)               \
  do {                                          \
    if (debug())                                \
      URBI_ROOT_ECHO(Self << ": " << S);        \
  } while (0)

# define URBI_ROOT_FATAL(Self, N, S)            \
  do {                                          \
    URBI_ROOT_ECHO(Self << ": " << S);          \
    exit(N);                                    \
  } while (0)



/*----------.
| Helpers.  |
`----------*/

static std::string
mygetenv(const std::string& var, const std::string& val)
{
  const char* res = getenv(var.c_str());
  return res ? std::string(res) : val;
}

static std::string
urbi_getenv(const std::string& logname,
            std::string var,
            const std::string& val = "")
{
  var = "URBI_" + var;
  const char* res = getenv(var.c_str());
  if (res)
  {
    URBI_ROOT_DEBUG(logname, "obeying to " << var << " = " << res);
    return res;
  }
  else
    return val;
}

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
# define RTLD_LAZY 0
# define RTLD_NOW 0
# define RTLD_GLOBAL 0

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
  return static_cast<void*>(GetProcAddress(module, name));
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
  size_t pos = lib.find(':');

  if ((pos = lib.find(':')) == lib.npos)
  {
    res.push_back(lib);
    return res;
  }

  do
  {
    std::string s = lib.substr(0, pos);
    if (pos != lib.npos)
    {
      lib = lib.substr(pos + 1, lib.npos);
      pos = lib.find(':');
    }
    else
      lib.clear();

#ifdef WIN32
    // Colon could be used as a path separator under cygwin and as a volume
    // separator under windows.
    if (s[0] == '\\' && !res.empty())
    {
      std::string& back = res.back();
      size_t back_len = back.length();

      // In case we split "c:\foo" into "c" and "\foo", glue them
      // together again.
      if (back_len == 1)
        back += ':' + s;

      // In case we split "bar;c:\foo" into "bar;c" and "\foo", split them
      // into "bar" and "c:\foo"
      else if (back_len >= 2 && back[back_len - 2] == ';')
      {
        s = std::string("") + back[back_len - 1] + ':' + s;
        back = back.substr(0, back_len - 2);
        res.push_back(s);
      }
    }
    else
      res.push_back(s);
#else
    res.push_back(s);
#endif

  } while (!lib.empty());

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
  strncpy(path, s.c_str(), sizeof path - 1);
  path[sizeof path - 1] = 0;
  while (readlink(path, path, sizeof path) != -1)
    URBI_ROOT_DEBUG(logname, "unrolling symbolic link: " << path);
  return path;
#endif
}

/// Find \a prog in $PATH.
/// \param logname  the prefix for log messages
/// \param prog     the program to look for (.exe will be appended on Windows)
/// \return  The full path to the parent of the directory that contains
///          \a prog, or "" if not found.
static
std::string
find_program(const std::string& logname,
             std::string prog)
{
#ifdef WIN32
  size_t pos = prog.find_last_of("/\\");
  {
    size_t dot = prog.find_last_of(".");
    if (dot == prog.npos || prog.substr(dot + 1) != "exe")
      prog += ".exe";
  }
#else
  size_t pos = prog.rfind('/');
#endif
  if (pos == std::string::npos)
  {
    struct stat stats;
    std::string dir, file;
    bool found = false;

#ifdef WIN32
    URBI_ROOT_DEBUG(logname,
                    "check if invoked from the current directory");

    {
      char *dir_buf = getcwd(0, 0);
      std::string dir(dir_buf);
      free(dir_buf);
      file = dir / prog;
    }

    found = stat(file.c_str(), &stats) == 0;
    if (!found)
    {
      URBI_ROOT_DEBUG(logname, "not found: " << file);
#endif
      URBI_ROOT_DEBUG(logname,
                      "check if invoked from the path");
      strings_type path = split(mygetenv("PATH"));
      foreach (const std::string& dir_, path)
      {
        file = dir_ / prog;
        found = stat(file.c_str(), &stats) == 0;
        if (found)
        {
          dir = dir_;
          break;
        }
        URBI_ROOT_DEBUG(logname, "not found: " << file);
      }
#ifdef WIN32
    }
#endif

    if (found)
    {
      URBI_ROOT_DEBUG(logname, "found: " << file);
      std::string res = dir / "..";
      URBI_ROOT_DEBUG(logname, "root directory is: " << res);
      return res;
    }
  }
  else
  {
    std::string res = prog.substr(0, pos) / "..";
    URBI_ROOT_DEBUG(logname,
                    "invoked with a path, setting root to parent directory: "
                    << res);
    return res;
  }
  return "";
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
  std::string uroot = urbi_getenv(program, "ROOT");
  if (uroot.empty())
  {
    URBI_ROOT_DEBUG(program_, "guessing Urbi root: invoked as: " << program_);
    // Handle the chained symlinks case.
    std::string argv0 = resolve_symlinks(program_, program);
    root_ = find_program(program_, argv0);
  }
  else
  {
    root_ = uroot;
    URBI_ROOT_DEBUG(program_,
                    "URBI_ROOT is set, forcing root directory: " << root_);
  }

  if (root_.empty())
    URBI_ROOT_FATAL(program_, 3,
                    "Unable to find Urbi SDK installation location. "
                    "Please set URBI_ROOT.");

  if (!static_build)
  {
    handle_libjpeg_      = library_load("jpeg4urbi");
    handle_libport_      = library_load("port");
    handle_libsched_     = library_load("sched");
#ifdef LIBPORT_ENABLE_SERIALIZATION
    handle_libserialize_ = library_load("serialize");
#endif
    handle_liburbi_      = library_load("urbi");
  }
}

RTLD_HANDLE
UrbiRoot::library_load(const std::string& base, const std::string& env_suffix)
{
  std::string envvar;
  if (env_suffix.empty())
    envvar = "ROOT_LIB" + base;
  else
    envvar = "ROOT_LIB" + env_suffix;
  foreach (char& s, envvar)
    s = toupper(s);

  return
    xdlopen(program_,
            base,
            urbi_getenv(program_, envvar,
                        root() / libdir / "lib" + base + library_suffix()));
}

const std::string&
UrbiRoot::root() const
{
  return root_;
}

std::string
UrbiRoot::core_path() const
{
  return root() / LIBPORT_LIBDIRNAME / "gostai";
}

std::string
UrbiRoot::doc_dir() const
{
  return urbi_getenv(program_, "DOC", root() / "share" / "doc" / "urbi-sdk");
}

std::string
UrbiRoot::share_dir() const
{
  return urbi_getenv(program_, "SHARE", root() / "share" / "gostai");
}

std::vector<std::string>
UrbiRoot::uobjects_path() const
{
  std::vector<std::string> res;
  if (!library_suffix().empty())
    res.push_back(core_path() / "uobjects" + library_suffix());
  res.push_back(core_path() / "uobjects");
  return res;
}

std::string
UrbiRoot::library_suffix() const
{
  return LIBPORT_LIBSFX;
}

void
UrbiRoot::load_plugin()
{
  handle_libuobject_ =
    xdlopen(program_,
            "plugin UObject implementation",
            urbi_getenv(program_, "ROOT_LIBPLUGIN",
                        core_path() / "engine" / "libuobject"+library_suffix()),
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
            urbi_getenv(program_, "ROOT_LIBREMOTE",
                        core_path() / "remote" / "libuobject"+library_suffix()),
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
#ifdef STATIC_BUILD
  return ::urbi_main_args(args, *this, block, errors);
#else
  urbi_main_type f =
    xdlsym<urbi_main_type>(program_,
                           "libuobject", handle_libuobject_,
                           "urbi_main_args");
  URBI_ROOT_DEBUG(program_, "command line: ");
  foreach (const std::string& arg, args)
    URBI_ROOT_DEBUG(program_, "  " << arg);

  return f(args, *this, block, errors);
#endif
}
