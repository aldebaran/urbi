#include <string>
#include <cassert>
#include <cstdarg>
#include <iostream>
#include <stdexcept>

#include <sdk/config.h>

#include <libltdl/ltdl.h>

#include <libport/cli.hh>
#include <libport/containers.hh>
#include <libport/file-system.hh>
#include <libport/foreach.hh>
#include <libport/path.hh>
#include <libport/package-info.hh>
#include <libport/program-name.hh>
#include <libport/sysexits.hh>
#include <libport/unistd.h>
#include <libport/windows.hh>

#include <urbi/package-info.hh>
#include <urbi/uclient.hh>

#include <boost/static_assert.hpp>
BOOST_STATIC_ASSERT(sizeof(unsigned long) == sizeof(void*));

using namespace urbi;
using libport::program_name;

// List of module names.
typedef std::list<std::string> modules_type;

namespace
{

  struct xlt_dladvise
  {
    lt_dladvise advise;

    xlt_dladvise()
    {
      if (lt_dladvise_init(&advise))
        std::cerr << program_name
                  << ": failed to initialize dladvise: "
                  << lt_dlerror() << std::endl
                  << libport::exit(1);
    }

    xlt_dladvise&
    global(bool global)
    {
      if (global ? lt_dladvise_global(&advise) : lt_dladvise_local(&advise))
        std::cerr << program_name << ": failed to set dladvise to "
                  << (global ? "global" : "local")
                  << ": "
                  << lt_dlerror() << std::endl
                  << libport::exit(1);
      return *this;
    }

    xlt_dladvise&
    ext()
    {
      if (lt_dladvise_ext(&advise))
        std::cerr << program_name << ": failed to set dladvise to ext: "
                  << lt_dlerror() << std::endl
                  << libport::exit(1);
      return *this;
    }
  };


  /// Wrapper around lt_dlopenext that exits on failures.
  static
  lt_dlhandle
  xlt_dlopenext(const std::string& s, bool global, int exit_failure = 1)
  {
    std::cerr << program_name
              << ": loading " << s << std::endl;
    lt_dlhandle res =
      lt_dlopenadvise(s.c_str(),
                      xlt_dladvise().global(global).ext().advise);
    if (!res)
      std::cerr << program_name << ": failed to load " << s
                << ": " << lt_dlerror() << std::endl
                << libport::exit(exit_failure);
    return res;
  }

  /// Wrapper around lt_dlsym that exits on failures.
  template <typename T>
  static
  T
  xlt_dlsym(lt_dlhandle h, const std::string& s)
  {
    void* res = lt_dlsym(h, s.c_str());
    if (!res)
      std::cerr << program_name
                << ": failed to dlsym " << s << ": " << lt_dlerror()
                << std::endl
                << libport::exit(1);
    // GCC 3.4.6 on x86_64 at least requires that we go through a
    // scalar type. It doesn't support casting a void* into a
    // function pointer directly. Later GCC versions do not have
    // this problem. We use a BOOST_STATIC_ASSERT at the top of
    // the file to ensure that "void*" and "unsigned long" have
    // the same size.
    return reinterpret_cast<T>(reinterpret_cast<unsigned long>(res));
  }

}

static UCallbackAction
onError(const UMessage& msg)
{
  std::cerr << program_name
            << ": load module error: " << msg.message << std::endl;
  return URBI_CONTINUE;
}

static UCallbackAction
onDone(const UMessage&)
{
  ::exit(0);
}

static int
connect_plugin(const std::string& host, int port, const modules_type& modules)
{
  UClient cl(host, port);
  if (cl.error())
    // UClient already displayed an error message.
    ::exit(1);
  cl.setErrorCallback(callback(&onError));
  cl.setCallback(callback(&onDone), "output");
  foreach (const std::string& m, modules)
    cl << "loadModule(\"" << m << "\");";
  cl << "output << 1;";
  while (true)
    sleep(1);
  return 0;
}

static void
usage()
{
  std::cout <<
    "usage: " << program_name << " [OPTIONS] MODULE_NAMES ... [-- UOPTIONS...]\n"
    "Start an UObject in either remote or plugin mode.\n"
    "\n"
    "Urbi-Launch options:\n"
    "  -h, --help         display this message and exit\n"
    "  -v, --version      display version information and exit\n"
    "  -c, --custom FILE  start using the shared library FILE\n"
    "\n"
    "Mode selection:\n"
    "  -p, --plugin       start as a plugin uobject on a running server\n"
    "  -r, --remote       start as a remote uobject\n"
    "  -s, --start        start an urbi server and connect as plugin\n"
    "\n"
    "Urbi-Launch options for plugin mode:\n"
    "  -H, --host            server host name\n"
    "  -P, --port            server port\n"
    "      --port-file FILE  file containing the port to listen to\n"
    "\n"
    "MODULE_NAMES is a list of modules.\n"
    "UOPTIONS are passed to urbi::main in remote and start modes.\n"
    "\n"
    "Exit values:\n"
    "  0  success\n"
    " " << EX_NOINPUT << "  some of the MODULES are missing\n"
    " " << EX_OSFILE << "  libuobject is missing\n"
    "  *  other kinds of errors\n"
    ;
  ::exit(EX_OK);
}


static
void
version()
{
  std::cout << urbi::package_info() << std::endl
            << libport::exit(EX_OK);
}

static
std::string
absolute(const std::string& s)
{
  libport::path p = s;
  if (!p.absolute_get())
    p = libport::get_current_directory() / p;
  return p.to_string();
}

static
int
ltdebug(unsigned verbosity, unsigned level, const char* format, va_list args)
{
  int errors = 0;
  if (level <= verbosity)
  {
    errors += fprintf(stderr, "%s: ", program_name.c_str()) < 0;
    errors += vfprintf(stderr, format, args) < 0;
  }
  return errors;
}

typedef int (*umain_type)(const libport::cli_args_type& args, bool block);
int
main(int argc, const char* argv[])
{
  program_name = argv[0];
  unsigned verbosity = 0;
  const char* urbi_root = getenv("URBI_ROOT");
  libport::path prefix(urbi_root ? urbi_root : URBI_PREFIX);

  enum ConnectMode
  {
    /// Start a new engine and plug the module
    MODE_PLUGIN_START,
    /// Load the module in a running engine as a plugin
    MODE_PLUGIN_LOAD,
    /// Connect the module to a running engine (remote uobject)
    MODE_REMOTE
  };
  ConnectMode connect_mode = MODE_REMOTE;
  /// Core dll to use.
  std::string dll;
  /// Server host name.
  std::string host = "localhost";
  /// Server port.
  int port = urbi::UClient::URBI_PORT;
  // The list of modules.
  modules_type modules;

  // The options passed to urbi::main.
  libport::cli_args_type args;
  args << argv[0];

  // Parse the command line.
  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];

    if (arg == "--custom" || arg == "-c")
      dll = libport::convert_argument<std::string> (arg, argv[++i]);
    else if (arg == "--debug")
      verbosity = libport::convert_argument<unsigned> (arg, argv[++i]);
    else if (arg == "--help" || arg == "-h")
      usage();
    else if (arg == "--host" || arg == "-H")
    {
      host = libport::convert_argument<std::string>(arg, argv[i+1]);
      args << argv[i] << argv[i+1];
      ++i;
    }
    else if (arg == "--plugin" || arg == "-p")
      connect_mode = MODE_PLUGIN_LOAD;
    else if (arg == "--port" || arg == "-P")
    {
      port = libport::convert_argument<int> (arg, argv[i+1]);
      args << argv[i] << argv[i+1];
      ++i;
    }
    else if (arg == "--port-file")
    {
      port =
        (libport::file_contents_get<int>
         (libport::convert_argument<const char*>(arg, argv[i+1])));
      args << argv[i] << argv[i+1];
      ++i;
    }
    else if (arg == "--remote" || arg == "-r")
      connect_mode = MODE_REMOTE;
    else if (arg == "--start" || arg == "-s")
      connect_mode = MODE_PLUGIN_START;
    else if (arg == "--version" || arg == "-v")
      version();
    else if (arg == "--")
    {
      for (int j = i + 1; j < argc; ++j)
        args << argv[j];
      break;
    }
    else if (arg[0] == '-' && arg[1] != 0)
      libport::invalid_option(arg);
    else
      // An argument: a module
      modules << absolute(arg);
  }

  if (connect_mode == MODE_PLUGIN_LOAD)
    return connect_plugin(host, port, modules);

  if (dll.empty())
    dll = prefix / "gostai" / "core" / URBI_HOST /
      (connect_mode == MODE_REMOTE ? "remote" : "engine") / "libuobject";

  /* The two other modes are handled the same way:
   * -Dlopen the correct libuobject.
   * -Dlopen the uobjects to load.
   * -Call urbi::main found by dlsym() in libuobject.
   */
  lt_dladd_log_function((lt_dllog_function*) &ltdebug, (void*) verbosity);
  lt_dlinit();
  lt_dlhandle core = xlt_dlopenext(dll, true, EX_OSFILE);
  foreach (const std::string& s, modules)
    xlt_dlopenext(s, false, EX_NOINPUT);

  umain_type umain = xlt_dlsym<umain_type>(core, "urbi_main_args");
  umain(args, true);
}
