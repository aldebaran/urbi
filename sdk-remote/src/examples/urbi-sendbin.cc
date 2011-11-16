/*
 * Copyright (C) 2005-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
   Send a binary file to the URBI server, to be saved in a variable.
*/

#include <vector>
#include <libport/sys/types.h>
#include <libport/cstdio>
#include <libport/sys/stat.h>

#include <libport/cli.hh>
#include <libport/debug.hh>
#include <libport/package-info.hh>
#include <libport/program-name.hh>
#include <libport/read-stdin.hh>
#include <libport/sysexits.hh>
#include <libport/unistd.h>
#include <libport/windows.hh>

#include <urbi/package-info.hh>
#include <urbi/uclient.hh>

using libport::program_name;

namespace
{
  static
  void
  usage()
  {
    std::cout <<
      "usage: " << program_name() << " [OPTION].. [VAR FILE HEADERS]...\n"
      "\n"
      "  VAR      variable name into which the value is stored\n"
      "  FILE     contents to store\n"
      "  HEADERS  associated headers\n"
      "\n"
      "For instance:\n"
      "  " << program_name() << " sounds.hello hello.wav WAV\n"
      "  " << program_name() << " \"var Global.hello\" hello.wav WAV\n"
      "\n"
      "Options:\n"
      "  -h, --help        output this message and exit successfully\n"
      "  -V, --version     output version information and exit successfully\n"
      "  -H, --host ADDR   the host running the Urbi server"
                << " [" << urbi::UClient::default_host() << "]\n"
      "  -p, --port PORT   the Urbi server port ["
                << urbi::UClient::URBI_PORT << "]\n"
      ;
    exit (EX_OK);
  }

  static
  void
  version()
  {
    std::cout << urbi::package_info() << std::endl
              << libport::exit(EX_OK);
  }
}


static urbi::UCallbackAction
dump(const urbi::UMessage& msg)
{
  // FIXME: This is absolutely not completely migrated.
  // To be finished -- Akim.
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


/// The triples given by the user.
struct data_type
{
  data_type(const char* v, const char* f, const char* h)
    : variable(v), file(f), headers(h)
  {}
  const char* variable;
  const char* file;
  const char* headers;
};

static
void
send_data(urbi::UClient& client, const data_type& data)
{
  try
  {
    std::string content = libport::read_file(data.file);
    client << data.variable << " = ";
    client.sendBinary(content.c_str(), content.size(), data.headers);
    client << ";\n";
    client.flush();
  }
  catch (const std::exception& e)
  {
    std::cerr << program_name() << ": " << e.what() << std::endl
              << libport::exit(EX_NOINPUT);
  }
}

GD_INIT();

int
main(int argc, char * argv[])
try
{
  libport::program_initialize(argc, argv);
  /// Server host name.
  std::string host = urbi::UClient::default_host();
  /// Server port.
  int port = urbi::UClient::URBI_PORT;
  /// Whether we send "quit" at the end.
  bool quit = false;

  std::vector<data_type> data;

  // Parse the command line.
  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];

    if (arg == "--help" || arg == "-h")
      usage();
    else if (arg == "--host" || arg == "-H")
      host = argv[++i];
    else if (arg == "--port" || arg == "-p")
      port = libport::convert_argument<int> (arg, argv[++i]);
    // FIXME: Remove -q some day.
    else if (arg == "--quit" || arg == "-Q" || arg == "-q")
      quit = true;
    // FIXME: Remove -v some day.
    else if (arg == "--version" || arg == "-V" || arg == "-v")
      version();
    else if (arg[0] == '-' && arg[1] != 0)
      libport::invalid_option (arg);
    else
    {
      if (argc < i + 3)
        libport::usage_error("too few arguments");
      data.push_back(data_type(argv[i], argv[i + 1], argv[i + 2]));
      i += 3;
    }
  }

  urbi::UClient client(host, port);
  client.setKeepAliveCheck(3000, 1000);
  if (client.error())
    std::cerr << libport::program_name() << ": client failed to set up"
	      << std::endl
              << libport::exit(1);

  client.setWildcardCallback(callback(&dump));
  client.setClientErrorCallback(callback(&error));
  client.waitForKernelVersion();

  /*----------------.
  | Send contents.  |
  `----------------*/
  for (std::vector<data_type>::const_iterator i = data.begin(),
         i_end = data.end();
       i != i_end;
       ++i)
    send_data(client, *i);
  if (quit)
    client.send("quit;");
  urbi::execute();
}
catch (const std::exception& e)
{
  std::cerr << program_name() << ": " << e.what() << std::endl
            << libport::exit(EX_FAIL);
}
