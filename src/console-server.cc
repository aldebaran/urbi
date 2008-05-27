//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include <libport/unistd.h>
#include <libport/sysexits.hh>
#include <libport/windows.hh>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <libport/cstring>

#include <libport/cli.hh>
#include <libport/exception.hh>
#include <libport/foreach.hh>
#include <libport/program-name.hh>
#include <libport/read-stdin.hh>
#include <libport/tokenizer.hh>
#include <libport/utime.hh>

// Inclusion order matters for windows. Leave userver.hh after network.hh.
#include <network/bsdnet/network.hh>
#include <kernel/userver.hh>
#include <kernel/uconnection.hh>

#include "ubanner.hh"

class ConsoleServer
  : public UServer
{
public:
  ConsoleServer(bool fast)
    : UServer("console"), fast(fast), ctime(0)
  {
    if (const char* cp = getenv ("URBI_PATH"))
    {
      std::string up(cp);
      std::list<std::string> paths;
      foreach (const std::string& s, libport::make_tokenizer(up, ":"))
      {
	if (s[0] == '\\' && paths.back().length() == 1)
	  paths.back() += ':' + s;
	else
	  paths.push_back(s);
      }
      foreach (const std::string& s, paths)
	search_path.append_dir(s);
    }
  }

  virtual ~ConsoleServer()
  {}

  virtual void shutdown()
  {
    UServer::shutdown ();
    exit (EX_OK);
  }
  virtual void beforeWork()
  {
  }
  virtual void reset()
  {}

  virtual void reboot()
  {}

  virtual libport::utime_t getTime()
  {
    return fast ? ctime : libport::utime();
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
  save_file(const std::string& filename, const std::string& content)
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
  libport::utime_t ctime;
};

namespace
{
  static
  void
  usage ()
  {
    std::cout <<
      "usage: " << libport::program_name << " [OPTIONS] [FILE...]\n"
      "\n"
      "  FILE    to load.  `-' stands for standard input\n"
      "\n"
      "Options:\n"
      "  -h, --help            display this message and exit successfully\n"
      "  -v, --version         display version information\n"
      "  -P, --period PERIOD   ignored for backward compatibility\n"
      "  -p, --port PORT       tcp port URBI will listen to.\n"
      "  -n, --no-network      disable networking.\n"
      "  -f, --fast            ignore system time, go as fast as possible\n"
      "  -i, --interactive     read and parse stdin in a nonblocking way\n"
      "  -w, --port-file FILE  write port number to specified file.\n"
      ;
    exit (EX_OK);
  }

  static
  void
  version ()
  {
    userver_package_info_dump(std::cout) << std::endl;
    exit (EX_OK);
  }
}

int
main (int argc, const char* argv[])
{
  libport::program_name = argv[0];

  // Input files.
  typedef std::vector<const char*> files_type;
  files_type files;
  /// The port to use.  0 means automatic selection.
  int arg_port = 0;
  /// Where to write the port we use.
  const char* arg_port_filename = 0;
  /// fast mode
  bool fast = false;
  /// interactive mode
  bool interactive = false;
  /// enable network
  bool network = true;
  // Parse the command line.
  {
    for (int i = 1; i < argc; ++i)
    {
      std::string arg = argv[i];

      if (arg == "--fast" || arg == "-f")
	fast = true;
      else if (arg == "--help" || arg == "-h")
	usage();
      else if (arg == "--interactive" || arg == "-i")
	interactive = true;
      else if (arg == "--no-network" || arg == "-n")
	network = false;
      else if (arg == "--period" || arg == "-P")
	(void) libport::convert_argument<int> (arg, argv[++i]);
      else if (arg == "--port" || arg == "-p")
	arg_port = libport::convert_argument<int> (arg, argv[++i]);
      else if (arg == "--port-file" || arg == "-w")
	arg_port_filename = argv[++i];
      else if (arg == "--version" || arg == "-v")
	version();
      else if (arg[0] == '-' && arg[1] != 0)
	libport::invalid_option (arg);
      else
	// An argument: a file.
        files.push_back(STREQ(argv[i], "-") ? "/dev/stdin" : argv[i]);
    }
  }

  ConsoleServer s(fast);
  int port = 0;
  if (network && !(port=Network::createTCPServer(arg_port, "localhost")))
    std::cerr << libport::program_name
	      << ": cannot bind to port " << arg_port
	      << " on localhost" << std::endl
	      << libport::exit (EX_UNAVAILABLE);

  if (arg_port_filename)
    std::ofstream(arg_port_filename, std::ios::out) << port << std::endl;


  s.initialize ();
  UConnection& c = s.getGhostConnection ();
  std::cerr << libport::program_name
	    << ": got ghost connection" << std::endl;

  foreach (const char* f, files)
    if (s.load_file(f, c.recv_queue_get ()) != USUCCESS)
      std::cerr << libport::program_name
		<< ": failed to process " << f << std::endl
		<< libport::exit(EX_NOINPUT);

  c.new_data_added_get() = true;

  std::cerr << libport::program_name << ": going to work..." << std::endl;
  libport::utime_t next_time = 0;
  while (true)
  {
    if (interactive)
    {
      std::string input;
      try
      {
	input = libport::read_stdin();
      }
      catch (libport::exception::Exception e)
      {
	std::cerr << e.what() << std::endl;
	interactive = false;
      }
      if (!input.empty())
	s.getGhostConnection().received(input.c_str(), input.length());
    }
    libport::utime_t select_time = 0;
    if (!fast)
    {
      libport::utime_t ctime = libport::utime();
      if (ctime < next_time)
	select_time = next_time - ctime;
      if (interactive)
	select_time = std::min(100000LL, select_time);
    }
    if (network)
      Network::selectAndProcess(select_time);

    next_time = s.work ();
    s.ctime = std::max (next_time, s.ctime + 1000L);
  }
}
