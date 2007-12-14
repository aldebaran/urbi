#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include "config.h"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include "libport/sysexits.hh"
#include "libport/windows.hh"
#include <fstream>

#include "libport/cli.hh"
#include "libport/utime.hh"

// Inclusion order matters for windows. Leave userver.hh after network.hh.
#include <network/bsdnet/network.hh>
#include "kernel/userver.hh"
#include "kernel/uconnection.hh"

#include "ubanner.hh"

const char* program_name;

class ConsoleServer
  : public UServer
{
public:
  ConsoleServer(int period)
    : UServer(period, 64000000, "console")
  {
    // FIXME: Add support for : in the path.
    if (const char* cp = getenv ("URBI_PATH"))
      path.push_back (cp);
  }

  virtual ~ConsoleServer()
  {}

  virtual void shutdown()
  {
    exit (0);
  }

  virtual void reset()
  {}

  virtual void reboot()
  {}

  virtual ufloat getTime()
  {
    return static_cast<ufloat>(libport::utime() / 1000LL);
  }

  virtual ufloat getPower()
  {
    return ufloat(1);
  }

  //! Called to display the header at each coonection start
  virtual void getCustomHeader(int line, char* header, int maxlength)
  {
    // FIXME: This interface is really really ridiculous and fragile.
    strncpy(header, uconsole_banner[line], maxlength);
  }

  virtual
  UErrorValue
  saveFile (const char* filename, const char* content)
  {
    //! \todo check this code
    std::ofstream os (filename);
    os << content;
    os.close ();
    return os.good () ? USUCCESS : UFAIL;
  }

  virtual
  void effectiveDisplay(const char* t)
  {
    std::cout << t;
  }
};

namespace
{
  static
  void
  usage ()
  {
    std::cout <<
      "usage: " << program_name << " [OPTIONS] [FILE]\n"
      "\n"
      "  FILE    to load\n"
      "\n"
      "Options:\n"
      "  -h, --help           display this message and exit successfully\n"
      "  -v, --version        display version information\n"
      "  -P, --period PERIOD  base URBI interval in milliseconds\n"
      "  -p, --port PORT      specify the tcp port URBI will listen to.\n"
      "  -w FILE              write port number to specified file.\n"
      ;
    exit (EX_OK);
  }

  static
  void
  version ()
  {
    userver_package_info_dump(std::cout) << std::endl;
    exit (0);
  }
}

int
main (int argc, const char* argv[])
{
  program_name = argv[0];

  // Input file.
  const char* in = "/dev/stdin";
  /// The period.
  int arg_period = 32;
  /// The port to use.  0 means automatic selection.
  int arg_port = 0;
  /// Where to write the port we use.
  const char* arg_port_filename = 0;

  // Parse the command line.
  {
    int argp = 1;
    for (int i = 1; i < argc; ++i)
    {
      std::string arg = argv[i];

      if (arg == "-h" || arg == "--help")
	usage();
      else if (arg == "--period" || arg == "-P")
	arg_period = libport::convert_argument<int> ("period", argv[++i]);
      else if (arg == "--port" || arg == "-p")
	arg_port = libport::convert_argument<int> (arg, argv[++i]);
      else if (arg == "-v" || arg == "--version")
	version();
      else if (arg == "-w")
	arg_port_filename = argv[++i];
      else if (arg[0] == '-')
	libport::invalid_option (arg);
      else
	// An argument.
	switch (argp++)
	{
	  case 1:
	    in = argv[i];
	    break;
	  default:
	    std::cerr << "Unexpected argument: " << arg << std::endl;
	    exit (1);
	    break;
	}
    }
  }

  int port = Network::createTCPServer(arg_port, "localhost");
  if (!port)
  {
    std::cerr << "cannot bind to port " << arg_port
	      << " on localhost" << std::endl;
    exit (EX_UNAVAILABLE);
  }
  if (arg_port_filename)
    std::ofstream(arg_port_filename, std::ios::out) << port << std::endl;
  Network::startNetworkProcessingThread();

  ConsoleServer s (arg_period);

  s.initialize ();
  UConnection& c = s.getGhostConnection ();
  DEBUG(("Got ghost connection\n"));

  if (s.loadFile(in, &c.recvQueue ()) != USUCCESS)
  {
    std::cerr << argv[0] << ": failed to process " << in << std::endl;
    return 1;
  }

  c.newDataAdded = true;

  long long startTime = 0;

  DEBUG(("Going to work...\n"));
  while (true)
  {
    startTime = libport::utime();
    ufloat period = s.getFrequency() * 1000;
    while (libport::utime() < startTime + period)
      usleep (1);
    s.work ();
  }
}
