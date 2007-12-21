//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include "config.h"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include "libport/sysexits.hh"
#include "libport/windows.hh"
#include <iostream>
#include <fstream>

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

#include "libport/cli.hh"
#include "libport/program-name.hh"
#include "libport/tokenizer.hh"
#include "libport/utime.hh"

// Inclusion order matters for windows. Leave userver.hh after network.hh.
#include <network/bsdnet/network.hh>
#include "kernel/userver.hh"
#include "kernel/uconnection.hh"

#include "ubanner.hh"

class ConsoleServer
  : public UServer
{
public:
  ConsoleServer(int period, bool fast)
    : UServer(period, "console"), fast(fast), ctime(0)
  {

    if (const char* cp = getenv ("URBI_PATH"))
    {
      std::string up(cp);
      BOOST_FOREACH (const std::string& s, libport::make_tokenizer(up, ":"))
        path.push_back (s);
    }
  }

  virtual ~ConsoleServer()
  {}

  virtual void shutdown()
  {
    UServer::shutdown ();
    exit (0);
  }
  virtual void beforeWork()
  {
    ctime += static_cast<long long>(period_get()) * 1000LL;
  }
  virtual void reset()
  {}

  virtual void reboot()
  {}

  virtual ufloat getTime()
  {
    if (fast)
      return ctime / 1000LL;
    else
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
  saveFile (const std::string& filename, const std::string& content)
  {
    //! \todo check this code
    std::ofstream os (filename.c_str ());
    os << content;
    os.close ();
    return os.good () ? USUCCESS : UFAIL;
  }

  virtual
  void effectiveDisplay(const char* t)
  {
    std::cout << t;
  }

  bool fast;
  long long ctime;
};

namespace
{
  static
  void
  usage ()
  {
    std::cout <<
      "usage: " << libport::program_name << " [OPTIONS] [FILE]\n"
      "\n"
      "  FILE    to load\n"
      "\n"
      "Options:\n"
      "  -h, --help           display this message and exit successfully\n"
      "  -v, --version        display version information\n"
      "  -P, --period PERIOD  base URBI interval in milliseconds\n"
      "  -p, --port PORT      specify the tcp port URBI will listen to.\n"
      "  -f, --fast           ignore system time, go as fast as possible\n"
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
  libport::program_name = argv[0];

  // Input file.
  const char* in = "/dev/stdin";
  /// The period.
  int arg_period = 32;
  /// The port to use.  0 means automatic selection.
  int arg_port = 0;
  /// Where to write the port we use.
  const char* arg_port_filename = 0;
  /// fast mode
  bool fast = false;
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
      else if (arg == "-f" || arg == "--fast")
	fast = true;
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
	    std::cerr << "Unexpected argument: " << arg << std::endl
		      << libport::exit (1);
	    break;
	}
    }
  }

  ConsoleServer s (arg_period, fast);

  int port = Network::createTCPServer(arg_port, "localhost");
  if (!port)
    std::cerr << "cannot bind to port " << arg_port
	      << " on localhost" << std::endl
	      << libport::exit (EX_UNAVAILABLE);

  if (arg_port_filename)
    std::ofstream(arg_port_filename, std::ios::out) << port << std::endl;
  Network::startNetworkProcessingThread();


  s.initialize ();
  UConnection& c = s.getGhostConnection ();
  DEBUG(("Got ghost connection\n"));

  if (s.loadFile(in, &c.recvQueue ()) != USUCCESS)
    std::cerr << argv[0] << ": failed to process " << in << std::endl
	      << libport::exit(1);

  c.newDataAdded = true;

  DEBUG(("Going to work...\n"));
  if (fast)
    while(true)
    {
      s.work();
    }
  else
    while (true)
    {
      long long startTime = libport::utime();
      ufloat period = s.period_get() * 1000;
      while (libport::utime() < startTime + period)
	usleep (1);
      s.work ();
    }
}
