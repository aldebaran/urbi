#include <ltdl.h>
#include <string>
#include <cassert>
#include <iostream>
#include <stdexcept>

#include <sdk/config.h>

#include <libport/cli.hh>
#include <libport/containers.hh>
#include <libport/file-system.hh>
#include <libport/foreach.hh>
#include <libport/path.hh>
#include <libport/program-name.hh>
#include <libport/sysexits.hh>
#include <libport/windows.hh>

#include <urbi/package-info.hh>
#include <urbi/uclient.hh>

using namespace urbi;
using libport::program_name;

// List of module names.
typedef std::list<std::string> modules_type;

// The kind of host (not the host name).
std::string urbi_host = URBI_HOST;
const char* umain_sym = "urbi_main_args";

namespace
{
  /// Wrapper around lt_dlopen that exits on failures.
  static
  lt_dlhandle
  xlt_dlopen(const std::string& s)
  {
    std::cerr << program_name << ": loading " << s << std::endl;
    lt_dlhandle res = lt_dlopen(s.c_str());
    if (!res)
      std::cerr << program_name
                << ": failed to load " << s << ": " << lt_dlerror() << std::endl
                << libport::exit(1);
    return res;
  }
}

static UCallbackAction
onError(const UMessage& msg)
{
  std::cerr <<"load module error: " << msg.message << std::endl;
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
  UClient cl(host.c_str(), port);
  if (cl.error())
    // UClient already displayed an error message.
    ::exit(1);
  cl.setErrorCallback(callback(&onError));
  cl.setCallback(callback(&onDone), "output");
  foreach(const std::string& m, modules)
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
    "  -h, --help        display this message and exit\n"
    "  -v, --version     display version information and exit\n"
    "\n"
    "Mode selection:\n"
    "  -c, --custom FILE  start using the shared library FILE\n"
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

static void
add_module(libport::path p, modules_type& res)
{
  if (!p.exists())
    throw std::runtime_error("File not found: " + p.to_string());

  const std::string dynld = SHLIBEXT;
  const std::string& base = p.components().back();
  if (base.size() <= dynld.size() ||
      base.substr(base.size() - dynld.size()) != dynld)
    throw std::runtime_error("File " + p.to_string() +
                             " does not look like a shared library " +
                             "(expect extension `" + dynld + "')");
  if (!p.absolute_get())
    p = libport::get_current_directory() / p;
  res.push_back(p.to_string());
}

typedef int (*umain_type)(const libport::cli_args_type& args, bool block);
int
main(int argc, const char* argv[])
{
  program_name = argv[0];
  lt_dlinit();

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
  ConnectMode connectMode = MODE_REMOTE;
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
    {
      connectMode = MODE_REMOTE;
      dll = libport::convert_argument<std::string> (arg, argv[++i]);
    }
    else if (arg == "--help" || arg == "-h")
      usage();
    else if (arg == "--host" || arg == "-H")
    {
      host = libport::convert_argument<std::string>(arg, argv[i+1]);
      args << argv[i] << argv[i+1];
      ++i;
    }
    else if (arg == "--plugin" || arg == "-p")
      connectMode = MODE_PLUGIN_LOAD;
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
      connectMode = MODE_REMOTE;
    else if (arg == "--start" || arg == "-s")
      connectMode = MODE_PLUGIN_START;
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
      add_module(argv[i], modules);
  }

  if (connectMode == MODE_PLUGIN_LOAD)
    return connect_plugin(host, port, modules);

  /* The two other modes are handled the same way:
   * -Dlopen the correct libuobject.
   * -Dlopen the uobjects to load.
   * -Call urbi::main found by dlsym() in libuobject.
   */
  if (dll.empty())
    dll = prefix / "gostai" / "core" / urbi_host /
      (connectMode == MODE_REMOTE ? "remote" : "engine") /
#ifdef WIN32
      "libuobject.dll"
#else
      "libuobject.so"
#endif
      ;
  lt_dlhandle core = xlt_dlopen(dll);

  foreach (const std::string& s, modules)
    xlt_dlopen(s);

  umain_type umain = (umain_type) lt_dlsym(core, umain_sym);
  if (!umain)
    std::cerr << "Failed to dlsym urbi::main: " << lt_dlerror() << std::endl
              << libport::exit(1);

  umain(args, true);
}
