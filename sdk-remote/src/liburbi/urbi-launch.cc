/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
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
#include <libport/foreach.hh>
#include <libport/package-info.hh>
#include <libport/program-name.hh>
#include <libport/sysexits.hh>
#include <libport/unistd.h>
#include <libport/windows.hh>
#include <libport/option-parser.hh>

#include <urbi/urbi-root.hh>

GD_CATEGORY(Urbi.UrbiLaunch);

using libport::program_name;

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
int
urbi_launch_(int argc, const char* argv[], UrbiRoot& urbi_root)
{
  libport::program_initialize(argc, argv);

  // The options passed to urbi::main.
  libport::cli_args_type args;

  args << argv[0];

  libport::OptionValue
    arg_custom("start using the shared library FILE", "custom", 'c', "FILE");
  libport::OptionFlag
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
    << arg_remote
    << arg_start
    << "Networking:"
    << libport::opts::host
    << libport::opts::port
    << libport::opts::port_file
    << arg_end;

  // The list of modules.
  libport::cli_args_type modules;
  try
  {
    modules = opt_parser(libport::program_arguments());
  }
  catch (const libport::Error& e)
  {
    foreach (std::string wrong_arg, e.errors())
      libport::invalid_option(wrong_arg);
  }

  if (libport::opts::help.get())
    usage(opt_parser);
  if (arg_root.get())
    std::cout << urbi_root.root() << std::endl << libport::exit(0);

  // Connection mode.
  enum ConnectMode
  {
    /// Start a new engine and plug the module
    MODE_PLUGIN_START,
    /// Connect the module to a running engine (remote uobject)
    MODE_REMOTE
  };

  ConnectMode connect_mode =
      arg_remote.get() ? MODE_REMOTE
    : arg_start.get()  ? MODE_PLUGIN_START
    :                    MODE_REMOTE;

  // Server host name.
  std::string host = libport::opts::host.value("127.0.0.1");
  if (libport::opts::host.filled())
    args << "--host" << host;

  // Server port.
  int port = libport::opts::port.get<int>(54000);
  if (libport::opts::port.filled())
    args << "--port" << libport::opts::port.value();

  if (libport::opts::port_file.filled())
  {
    std::string file = libport::opts::port_file.value();
    args << "--port-file" << file;
  }

  foreach (const std::string& s, modules)
    args << "--module" << s;

  // Other arguments, after `--'.
  args.insert(args.end(), arg_end.get().begin(), arg_end.get().end());
  if (libport::opts::version.get())
    args.push_back("--version");

  // Open the right core library.
  if (arg_custom.filled())
    urbi_root.load_custom(arg_custom.value());
  else if (connect_mode == MODE_REMOTE)
    urbi_root.load_remote();
  else
    urbi_root.load_plugin();
  return urbi_root.urbi_main(args, true, true);
}

extern "C"
{
  int
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
