/*
 * Copyright (C) 2005-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \brief Send commands to an Urbi server.

#include <libport/bind.hh>
#include <libport/cli.hh>
#include <libport/debug.hh>
#include <libport/cstdio>
#include <libport/foreach.hh>
#include <libport/input-arguments.hh>
#include <libport/option-parser.hh>
#include <libport/package-info.hh>
#include <libport/program-name.hh>
#include <libport/sysexits.hh>
#include <list>
#include <string>

#include <urbi/package-info.hh>
#include <urbi/uclient.hh>

using libport::program_name;

namespace
{
  static
  void
  usage(libport::OptionParser& parser)
  {
    std::cout <<
      "usage: " << program_name() << " [OPTION].. [FILE]...\n"
      "\n"
      "  FILE    to upload onto the server\n"
                << parser
                << "\n"
                << urbi::package_info().report_bugs()
                << std::endl
                << libport::exit(EX_OK);
  }

  static
  void
  version()
  {
    std::cout << "urbi-send" << std::endl
              << urbi::package_info() << std::endl
              << libport::exit(EX_OK);
  }
}

static urbi::UCallbackAction
dump(const urbi::UMessage& msg)
{
  switch (msg.type)
  {
    case urbi::MESSAGE_DATA:
    case urbi::MESSAGE_SYSTEM:
      std::cout << msg << std::endl;
      break;

    case urbi::MESSAGE_ERROR:
      std::cerr << msg << std::endl;
      break;
  }
  return urbi::URBI_CONTINUE;
}

static urbi::UCallbackAction
error(const urbi::UMessage& msg)
{
  dump(msg);
  exit(0);
}


struct DataSender : libport::opts::DataVisitor
{
  typedef libport::opts::DataVisitor super_type;
  typedef urbi::UAbstractClient::error_type error_type;

  DataSender(urbi::UClient& u)
    : client_(u)
    , error_(0)
  {}

  using super_type::operator();

  virtual
  void
  operator()(const libport::opts::Data& d)
  {
    super_type::operator()(d);
    if (error_)
    {
      std::cerr << libport::program_name() << ": failed to send " << d;
      if (errno)
        std::cerr << ": " << strerror(errno);
      std::cerr << std::endl << libport::exit(1);
    }
  }

  virtual
  void
  operator()(const libport::opts::TextData& d)
  {
    error_ = client_.send("%s", d.command_.c_str());
  }

  virtual
  void
  operator()(const libport::opts::FileData& d)
  {
    error_ = client_.sendFile(d.filename_);
  }

  urbi::UClient& client_;
  error_type error_;
};

/*-------.
| Main.  |
`-------*/

GD_INIT();

int
main(int argc, char* argv[])
try
{
  libport::program_initialize(argc, argv);

  // Parse the command line.
  libport::OptionFlag
    arg_quit("disconnect when everything is sent", "quit", 'Q');

  libport::OptionParser opt_parser;
  opt_parser << "Options:"
	     << libport::opts::exp
	     << libport::opts::file
	     << libport::opts::module
	     << libport::opts::help
	     << libport::opts::host
	     << libport::opts::port
	     << libport::opts::port_file
	     << libport::opts::version
             << arg_quit;

  libport::cli_args_type
    remainings_args = opt_parser(libport::program_arguments());

  if (libport::opts::help.get())
    usage(opt_parser);
  if (libport::opts::version.get())
    version();

  /// Server host name.
  std::string host = libport::opts::host.value(urbi::UClient::default_host());
  /// Server port.
  int port = libport::opts::port.get<int>(urbi::UClient::URBI_PORT);
  if (libport::opts::port_file.filled())
    port = libport::file_contents_get<int>(libport::opts::port_file.value());

  foreach(const std::string& arg, remainings_args)
    if (arg[0] == '-' && arg[1] != 0)
      libport::invalid_option(arg);
    else
      libport::opts::input_arguments.add_file(arg);

  urbi::UClient client(host, port);
  client.setKeepAliveCheck(3000, 1000);
  if (client.error())
    std::cerr << libport::program_name() << ": client failed to set up"
	      << std::endl
              << libport::exit(1);

  client.setWildcardCallback(callback(&dump));
  client.setClientErrorCallback(callback(&error));

  /*----------------.
  | Send contents.  |
  `----------------*/
  DataSender send(client);
  send(libport::opts::input_arguments);
  libport::opts::input_arguments.clear();

  if (arg_quit.get())
    client.send("quit;");
  else
    std::cout << libport::program_name()
              << ": hit Ctrl-C to terminate."
              << std::endl;
  urbi::execute();
}
catch (const std::exception& e)
{
  std::cerr << program_name() << ": " << e.what() << std::endl
            << libport::exit(EX_FAIL);
}
