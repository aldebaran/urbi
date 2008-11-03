#include <ltdl.h>
#include <string>
#include <cassert>
#include <iostream>
#include <stdexcept>

#include <sdk/config.h>

#include <libport/cli.hh>
#include <libport/file-system.hh>
#include <libport/foreach.hh>
#include <libport/path.hh>
#include <libport/program-name.hh>
#include <libport/sysexits.hh>
#include <libport/windows.hh>

#include <urbi/package-info.hh>
#include <urbi/uclient.hh>

using namespace urbi;

// List of module names.
typedef std::list<std::string> modules_type;

// The kind of host (not the host name).
std::string urbi_host = URBI_HOST;
std::string dynld = URBI_SHREXT;
const char* umain_sym = "urbi_main";

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
connect_plugin(const std::string& host, int port, modules_type& modules)
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
    "usage: " << libport::program_name << " [OPTIONS] MODULE_NAMES ...\n"
    "    Start an UObject in either remote or plugin mode\n"
    "\n"
    "Options:\n"
    "  -h, --help        display this message and exit\n"
    "  -v, --version     display version information and exit\n"
    "\n"
    "Mode selection:\n"
    "  -r, --remote       start as a remote uobject\n"
    "  -p, --plugin       start as a plugin uobject on a running server\n"
    "  -s, --start        start an urbi server and connect as plugin\n"
    "  -c, --custom FILE  start using the shared library FILE\n"
    "\n"
    "Options for plugin mode:\n"
    "  -H, --host   server host name\n"
    "  -p, --port   server port\n"
    "\n"
    "Options for remote and start mode are passed to urbi::main.\n"
    "\n"
    "MODULE_NAMES is a list of modules and directory which will be searched\n"
    "  for modules.\n"
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
  if (p.exists())
  {
    const libport::path::path_type& components = p.components();
    const std::string& last = components.back();
    if (last.size() <= dynld.size() ||
	last.substr(last.size() - dynld.size()) != dynld)
      throw std::runtime_error("File " + p.to_string() +
			       " does not look like a shared library " +
			       "(extension `" + dynld + "')");
    if (!p.absolute_get())
      p = libport::get_current_directory() / p;
    res.push_back(p.to_string());
  }
  else
    throw std::runtime_error("File not found: " + p.to_string());
}

typedef int (*umain_type)(int argc, const char* argv[], int block);
int
main(int argc, const char* argv[])
{
  libport::program_name = argv[0];
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

  // Parse the command line.
  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];

    if (arg == "--custom" || arg == "-c")
      connectMode = MODE_REMOTE;
    else if (arg == "--help" || arg == "-h")
      usage();
    else if (arg == "--host" || arg == "-H")
      host = argv[++i];
    else if (arg == "--plugin" || arg == "-p")
      connectMode = MODE_PLUGIN_LOAD;
    else if (arg == "--remote" || arg == "-r")
    {
      connectMode = MODE_REMOTE;
      dll = libport::convert_argument<std::string> (arg, argv[++i]);
    }
    else if (arg == "--start" || arg == "-s")
      connectMode = MODE_PLUGIN_START;
    else if (arg == "--version" || arg == "-v")
      version();
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
  std::cerr << "loading core: " << dll << std::endl;
  lt_dlhandle core = lt_dlopen(dll.c_str());
  if (!core)
  {
    std::cerr << "Failed to load core: " << lt_dlerror() << std::endl;
    ::exit(1);
  }

  foreach (const std::string& s, modules)
  {
    std::cerr << "Loading uobjects: " << s << std::endl;
    lt_dlhandle uobject = lt_dlopen(s.c_str());
    if (!uobject)
      std::cerr << "Failed to load " << s << ": " << lt_dlerror() << std::endl
                << libport::exit(1);
  }

  umain_type umain = (umain_type) lt_dlsym(core, umain_sym);
  if (!umain)
    std::cerr << "Failed to dlsym urbi::main: " << lt_dlerror() << std::endl
              << libport::exit(1);
  umain(argc, argv, true);
}
