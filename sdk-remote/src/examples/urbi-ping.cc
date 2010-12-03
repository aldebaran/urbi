/*
 * Copyright (C) 2005-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cli.hh>
#include <libport/csignal>
#include <libport/option-parser.hh>
#include <libport/package-info.hh>
#include <libport/program-name.hh>
#include <libport/sys/stat.h>
#include <libport/sys/types.h>
#include <libport/sysexits.hh>
#include <libport/unistd.h>
#include <libport/windows.hh>

#include <urbi/package-info.hh>
#include <urbi/uclient.hh>

using libport::program_name;

unsigned sendtime;
unsigned mintime;
unsigned maxtime;
unsigned sumtime = 0;
unsigned pingCount = 0;
bool over = false;
std::string host;
bool received;
unsigned count;
bool flood = false;

namespace
{
  static
  void
  usage(libport::OptionParser& parser)
  {
    std::cout <<
      "usage: " << program_name() << " [HOST] [INTERVAL] [COUNT]\n"
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
    std::cout << "urbi-ping" << std::endl
              << urbi::package_info() << std::endl
              << libport::exit(EX_OK);
  }
}


static urbi::UCallbackAction
pong(const urbi::UMessage& msg)
{
  unsigned int ptime = msg.client.getCurrentTime() - sendtime;
  if (!pingCount || ptime < mintime)
    mintime = ptime;
  if (!pingCount || maxtime < ptime)
    maxtime = ptime;

  sumtime += ptime;
  printf("ping reply from %s: seq=%d time=%d ms\n",
         host.c_str(), pingCount, ptime);
  ++pingCount;
  received = true;
  if (pingCount == count)
    over = true;
  if (flood)
  {
    sendtime = msg.client.getCurrentTime();
    msg.client.send("ping,\n");
  }
  return urbi::URBI_CONTINUE;
}

static void showstats(int)
{
  if (pingCount)
    printf("rtt min/avg/max %d/%d/%d ms\n",
           (mintime),
           (int)(sumtime/(float)pingCount),
           (maxtime));
  exit(0);
}


int
main(int argc, char* argv[])
try
{
  signal(SIGINT, showstats);

  libport::program_initialize(argc, argv);

  libport::OptionValue
    arg_interval("ping interval, in milliseconds. 0 to flood (1000)",
                 "interval", 'i', "INTERVAL"),
    arg_count("number of pings to send, 0 for unbound (0)",
              "count", 'c', "COUNT");

  // Parse the command line.
  libport::OptionParser opt_parser;
  opt_parser << "Options:"
             << arg_count
	     << libport::opts::help
	     << libport::opts::host
             << arg_interval
	     << libport::opts::port
	     << libport::opts::port_file
	     << libport::opts::version;

  libport::cli_args_type args = opt_parser(libport::program_arguments());

  foreach(const std::string& arg, args)
    if (arg[0] == '-')
      libport::invalid_option(arg);
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

  count = arg_count.get<unsigned>(0u);
  unsigned interval = arg_interval.get<unsigned>(1000u);
  switch (args.size())
  {
  case 3: count = libport::convert_argument<unsigned>("count", args[2]);
  case 2: interval = libport::convert_argument<unsigned>("interval", args[1]);
  case 1: host = args[0];
  case 0: break;
  default:
    libport::usage_error("invalid numer of arguments");
  }

  // Client initialization.
  urbi::UClient client(host, port);
  if (client.error())
    std::cerr << libport::program_name() << ": client failed to set up"
	      << std::endl
              << libport::exit(1);
  client.start();

  client.send("var uping = Channel.new(\"uping\")|;\n"
              "function ping () { uping << \"ping\"}|;\n");

  client.setCallback(&pong, "uping");

  received = true;

  if (interval)
    for (unsigned i = 0; i < count || !count; ++i)
    {
      while (!received)
	usleep(200);
      received = false;
      sendtime = client.getCurrentTime();
      client.send("ping,\n");
      usleep(interval*1000);
    }
  else
  {
    flood = true;
    client.send("ping,\n");
  }

  while (!over)
    usleep(1000000);
  showstats(0);
}
catch (const std::exception& e)
{
  std::cerr << program_name() << ": " << e.what() << std::endl
            << libport::exit(EX_FAIL);
}
