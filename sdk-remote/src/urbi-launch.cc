#include <string>
#include <cassert>
#include <cstdarg>
#include <libport/cstdlib>
#include <iostream>
#include <stdexcept>

#include <sdk/config.h>

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
#include <libport/option-parser.hh>
#include <libport/xltdl.hh>

#include <urbi/exit.hh>
#include <urbi/package-info.hh>
#include <urbi/uclient.hh>

#include <boost/static_assert.hpp>
BOOST_STATIC_ASSERT(sizeof(unsigned long) == sizeof(void*));

using namespace urbi;
using libport::program_name;

// List of module names.
typedef std::list<std::string> modules_type;

static UCallbackAction
onError(const UMessage& msg)
{
  std::cerr << program_name()
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
usage(libport::OptionParser& parser)
{
  std::cout <<
    "usage: " << program_name() <<
    " [OPTIONS] MODULE_NAMES ... [-- UOPTIONS...]\n"
    "Start an UObject in either remote or plugin mode.\n"
              << parser <<
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
    errors += fprintf(stderr, "%s: ", program_name().c_str()) < 0;
    errors += vfprintf(stderr, format, args) < 0;
  }
  return errors;
}

typedef int (*umain_type)(const libport::cli_args_type& args,
                          bool block, bool errors);
int
main(int argc, char* argv[])
{
  libport::program_initialize(argc, argv);
  unsigned verbosity = 0;
  libport::path urbi_root = libport::xgetenv("URBI_ROOT", URBI_ROOT);

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
  libport::cli_args_type args4modules;
  args << argv[0];

  libport::OptionValue
    arg_custom("start using the shared library FILE", "custom", 'c', "FILE"),
    arg_debug("increase verbosity for debug", "debug", 0, "N"),
    arg_pfile("file containing the port to listen to", "port-file", 0, "FILE");
  libport::OptionFlag
    arg_plugin("start as a plugin uobject on a running server", "plugin", 'p'),
    arg_remote("start as a remote uobject", "remote", 'r'),
    arg_start("start an urbi server and connect as plugin", "start", 's');
  libport::OptionsEnd arg_end;

  libport::OptionParser opt_parser;
  opt_parser << "Urbi-Launch options:"
	     << libport::opts::help
	     << libport::opts::version
	     << arg_custom
	     << arg_debug
	     << "Mode selection:"
	     << arg_plugin
	     << arg_remote
	     << arg_start
	     << "Urbi-Launch options for plugin mode:"
	     << libport::opts::host
	     << libport::opts::port
	     << arg_pfile
	     << arg_end;

  try
  {
    args4modules = opt_parser(libport::program_arguments());
  }
  catch (libport::Error& e)
  {
    const libport::Error::errors_type& err = e.errors();
    foreach (std::string wrong_arg, err)
      libport::invalid_option(wrong_arg);
  }
  foreach (std::string& mod_arg, args4modules)
    modules << absolute(mod_arg);

  if (libport::opts::version.get())
    version();
  if (libport::opts::help.get())
    usage(opt_parser);
  if (arg_custom.filled())
    dll = arg_custom.value();
  if (arg_debug.filled())
    verbosity = arg_debug.get<unsigned>(1);
  if (libport::opts::host.filled())
  {
    host = libport::opts::host.value();
    args << "--host" << host;
  }
  if (arg_plugin.get())
    connect_mode = MODE_PLUGIN_LOAD;
  if (libport::opts::port.filled())
  {
    port = libport::opts::port.get<int>();
    args << "--port" << libport::opts::port.value();
  }
  if (arg_pfile.filled())
  {
    std::string my_arg = arg_pfile.value();
    port = libport::file_contents_get<int>(my_arg);
    args << "--port-file" << my_arg;
  }
  if (arg_remote.get())
    connect_mode = MODE_REMOTE;
  if (arg_start.get())
    connect_mode = MODE_PLUGIN_START;
  args.insert(args.end(), arg_end.get().begin(), arg_end.get().end());

  if (connect_mode == MODE_PLUGIN_LOAD)
    return connect_plugin(host, port, modules);

  if (dll.empty())
    dll = urbi_root / "gostai" / "core" / URBI_HOST /
      (connect_mode == MODE_REMOTE ? "remote" : "engine") / "libuobject";

  /* The two other modes are handled the same way:
   * -Dlopen the correct libuobject.
   * -Dlopen the uobjects to load.
   * -Call urbi::main found by dlsym() in libuobject.
   */
  lt_dladd_log_function((lt_dllog_function*) &ltdebug, (void*) verbosity);
  lt_dlinit();
  lt_dlhandle core = libport::xlt_dlopenext(dll, true, EX_OSFILE, verbosity);
  foreach (const std::string& s, modules)
    libport::xlt_dlopenext(s, false, EX_NOINPUT);

  umain_type umain = libport::xlt_dlsym<umain_type>(core, "urbi_main_args");
  umain(args, true, true);
}
