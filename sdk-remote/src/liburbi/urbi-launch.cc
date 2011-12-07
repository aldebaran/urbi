/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <string>
#include <libport/cassert>
#include <cstdarg>
#include <libport/cstdlib>
#include <iostream>
#include <stdexcept>

#include <boost/assign/list_of.hpp>
using namespace boost::assign;

#include <libport/cli.hh>
#include <libport/containers.hh>
#include <libport/debug.hh>
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
#include <urbi/urbi-root.hh>

GD_CATEGORY(Urbi.UrbiLaunch);

using namespace urbi;
using libport::program_name;

static UCallbackAction
onError(const UMessage& msg)
{
  LIBPORT_USE(msg);
  GD_FERROR("%s: load module error: %s", program_name(), msg.message);
  return URBI_CONTINUE;
}

static UCallbackAction
onDone(const UMessage&)
{
  ::exit(0);
}

static int
connect_plugin(const std::string& host, int port,
               const libport::cli_args_type& modules)
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
int
urbi_launch_(int argc, const char* argv[], UrbiRoot& urbi_root)
{
  libport::program_initialize(argc, argv);

  // The options passed to urbi::main.
  libport::cli_args_type args;

  args << argv[0];

  libport::OptionValue
    arg_custom("start using the shared library FILE", "custom", 'c', "FILE"),
    arg_pfile("file containing the port to listen to", "port-file", 0, "FILE");
  libport::OptionFlag
    arg_plugin("start as a plugin uobject on a running server", "plugin", 'p'),
    arg_remote("start as a remote uobject", "remote", 'r'),
    arg_root("output Urbi root and exit", "print-root"),
    arg_start("start an urbi server and connect as plugin", "start", 's');
  libport::OptionsEnd arg_end;

  libport::OptionParser opt_parser;
  opt_parser << "Options:"
	     << libport::opts::help
	     << libport::opts::version
             << arg_root
	     << arg_custom
#ifndef LIBPORT_DEBUG_DISABLE
	     << libport::opts::debug
#endif
	     << "Mode selection:"
	     << arg_plugin
	     << arg_remote
	     << arg_start
	     << "Networking:"
	     << libport::opts::host
	     << libport::opts::port
	     << arg_pfile
	     << arg_end;

  // The list of modules.
  libport::cli_args_type modules;
  try
  {
    modules = opt_parser(libport::program_arguments());
  }
  catch (const libport::Error& e)
  {
    const libport::Error::errors_type& err = e.errors();
    foreach (std::string wrong_arg, err)
      libport::invalid_option(wrong_arg);
  }

  if (libport::opts::version.get())
    version();
  if (libport::opts::help.get())
    usage(opt_parser);
  if (arg_root.get())
    std::cout << urbi_root.root() << std::endl << libport::exit(0);

  // Connection mode.
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

  if (arg_plugin.get())
    connect_mode = MODE_PLUGIN_LOAD;
  if (arg_remote.get())
    connect_mode = MODE_REMOTE;
  if (arg_start.get())
    connect_mode = MODE_PLUGIN_START;

  /// Server host name.
  std::string host = libport::opts::host.value(UClient::default_host());
  if (libport::opts::host.filled())
    args << "--host" << host;

  /// Server port.
  int port = libport::opts::port.get<int>(urbi::UClient::URBI_PORT);
  if (libport::opts::port.filled())
    args << "--port" << libport::opts::port.value();

  if (arg_pfile.filled())
  {
    std::string file = arg_pfile.value();
    if (connect_mode == MODE_PLUGIN_LOAD)
      port = libport::file_contents_get<int>(file);
    args << "--port-file" << file;
  }
  args.insert(args.end(), arg_end.get().begin(), arg_end.get().end());

  if (connect_mode == MODE_PLUGIN_LOAD)
    return connect_plugin(host, port, modules);

  // Open the right core library.
  if (arg_custom.filled())
    urbi_root.load_custom(arg_custom.value());
  else if (connect_mode == MODE_REMOTE)
    urbi_root.load_remote();
  else
    urbi_root.load_plugin();

  // If URBI_UOBJECT_PATH is not defined, first look in ., then in the
  // stdlib.
  std::string uobject_path = libport::xgetenv("URBI_UOBJECT_PATH", ".:");

  // Load the modules using our uobject library path.
  libport::xlt_advise dl;
  dl.ext().path().push_back(uobject_path, ":");
  foreach(const std::string& s, urbi_root.uobjects_path())
    dl.path().push_back(s);
  foreach (const std::string& s, modules)
    dl.open(s);

  return urbi_root.urbi_main(args, true, true);
}

extern "C"
{
  int
  URBI_SDK_API
  urbi_launch(int argc, const char* argv[], UrbiRoot& root)
    try
    {
      return urbi_launch_(argc, argv, root);
    }
    catch (const std::exception& e)
    {
      std::cerr << argv[0] << ": " << e.what() << std::endl
                << libport::exit(EX_FAIL);
    }
}
