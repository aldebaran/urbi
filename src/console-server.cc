//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include <libport/unistd.h>
#include <libport/sysexits.hh>
#include <libport/windows.hh>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <libport/cstring>

#include <libport/cli.hh>
#include <libport/exception.hh>
#include <libport/foreach.hh>
#include <libport/program-name.hh>
#include <libport/read-stdin.hh>
#include <libport/sys/socket.h>
#include <libport/thread.hh>
#include <libport/tokenizer.hh>
#include <libport/utime.hh>

// Inclusion order matters for windows. Leave userver.hh after network.hh.
#include <network/bsdnet/network.hh>
#include <kernel/kernconf.hh>
#include <kernel/userver.hh>
#include <kernel/uconnection.hh>

#include <scheduler/scheduler.hh>

#include <kernel/ubanner.hh>
#include <urbi/export.hh>
#include <urbi/umain.hh>
#include <urbi/uobject.hh>

class ConsoleServer
  : public UServer
{
public:
  ConsoleServer(bool fast)
    : UServer("console"), fast(fast), ctime(0)
  {
  }

  virtual ~ConsoleServer()
  {}

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
  virtual void
  getCustomHeader(unsigned int line, char* header, size_t maxlength)
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
      "usage: " << libport::program_name << " [OPTION].. [FILE]...\n"
      "\n"
      "  FILE    to load.  `-' stands for standard input\n"
      "\n"
      "Options:\n"
      "  -h, --help             display this message and exit successfully\n"
      "  -v, --version          display version information\n"
      "  -P, --period PERIOD    ignored for backward compatibility\n"
      "  -H, --host HOST        the address to listen on (default: all)\n"
      "  -p, --port PORT        tcp port URBI will listen to\n"
      "  -n, --no-network       disable networking\n"
      "  -f, --fast             ignore system time, go as fast as possible\n"
      "  -i, --interactive      read and parse stdin in a nonblocking way\n"
      "  -s, --stack-size=SIZE  set the job stack size in KB\n"
      "  -w, --port-file FILE   write port number to specified file.\n"
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

namespace urbi {


/// Command line options needed in the main loop.
struct LoopData
{
  bool interactive;
  bool fast;
  bool network;
  ConsoleServer* s;
};

int main_loop(LoopData& l);

USDK_API int
main (int argc, const char* argv[], bool block)
{
  libport::program_name = argv[0];
  LoopData data;
  // Input files.
  typedef std::vector<const char*> files_type;
  files_type files;
  /// The IP to bind. "" means every interface.
  std::string arg_host;
  /// The port to use.  0 means automatic selection.
  int arg_port = 0;
  /// Where to write the port we use.
  const char* arg_port_filename = 0;
  /// The size of the stacks.
  size_t arg_stack_size = 0;
  /// fast mode
  data.fast = false;
  /// interactive mode
  data.interactive = false;
  /// enable network
  data.network = true;

  // Parse the command line.
  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];

    if (arg == "--fast" || arg == "-f")
      data.fast = true;
    else if (arg == "--help" || arg == "-h")
      usage();
    else if (arg == "--host" || arg == "-H")
      arg_host = argv[++i];
    else if (arg == "--interactive" || arg == "-i")
      data.interactive = true;
    else if (arg == "--no-network" || arg == "-n")
      data.network = false;
    else if (arg == "--period" || arg == "-P")
      (void) libport::convert_argument<int> (arg, argv[++i]);
    else if (arg == "--port" || arg == "-p")
      arg_port = libport::convert_argument<int> (arg, argv[++i]);
    else if (arg == "--port-file" || arg == "-w")
      arg_port_filename = argv[++i];
    else if (arg == "--stack-size" || arg == "-s")
      arg_stack_size = libport::convert_argument<size_t> (arg, argv[++i]);
    else if (arg == "--version" || arg == "-v")
      version();
    else if (arg[0] == '-' && arg[1] != 0)
      libport::invalid_option (arg);
    else
      // An argument: a file.
      files.push_back(STREQ(argv[i], "-") ? "/dev/stdin" : argv[i]);
  }

  // If not defined in line, use the envvar.
  if (!arg_stack_size
      && getenv("URBI_STACK_SIZE"))
    arg_stack_size = libport::convert_envvar<size_t> ("URBI_STACK_SIZE");

  if (arg_stack_size)
  {
    // Make sure the result is a multiple of the page size.  This
    // required at least on OSX (which unfortunately fails with errno
    // = 0).
    arg_stack_size *= 1024;
    size_t pagesize = getpagesize();
    arg_stack_size = ((arg_stack_size + pagesize - 1) / pagesize) * pagesize;
    kernconf.default_stack_size = arg_stack_size;
  }

  data.s = new ConsoleServer(data.fast);
  ConsoleServer& s = *data.s;
  int port = 0;
  if (data.network && !(port=Network::createTCPServer(arg_port, arg_host)))
  {
    std::cerr << libport::program_name
	      << ": cannot bind to port " << arg_port;
    if (!arg_host.empty())
      std::cerr << " on " << arg_host;
    std::cerr << std::endl
	      << libport::exit (EX_UNAVAILABLE);
  }

  s.initialize ();

  // Write the port file after initialize returned; that is, after
  // urbi.u is loaded.
  if (arg_port_filename)
    std::ofstream(arg_port_filename, std::ios::out) << port << std::endl;

  UConnection& c = s.ghost_connection_get();
  std::cerr << libport::program_name
	    << ": got ghost connection" << std::endl;

  foreach (const char* f, files)
    if (s.load_file(f, c.recv_queue_get ()) != USUCCESS)
      std::cerr << libport::program_name
		<< ": failed to process " << f << std::endl
		<< libport::exit(EX_NOINPUT);

  c.new_data_added_get() = true;

  std::cerr << libport::program_name << ": going to work..." << std::endl;
  if (block)
    return main_loop(data);
  else
    libport::startThread(new boost::function0<void>(
      boost::bind(&main_loop, data)));
  return 0;
  }

int main_loop(LoopData& data)
{
  ConsoleServer& s = *data.s;
  libport::utime_t next_time = 0;
  while (true)
  {
    if (data.interactive)
    {
      std::string input;
      try
      {
	input = libport::read_stdin();
      }
      catch (libport::exception::Exception e)
      {
	std::cerr << e.what() << std::endl;
	data.interactive = false;
      }
      if (!input.empty())
	s.ghost_connection_get().received(input.c_str(), input.length());
    }
    libport::utime_t select_time = 0;
    if (!data.fast)
    {
      libport::utime_t ctime = libport::utime();
      if (ctime < next_time)
	select_time = next_time - ctime;
      if (data.interactive)
	select_time = std::min(100000LL, select_time);
    }
    if (data.network)
      Network::selectAndProcess(select_time);

    next_time = s.work ();
    if (next_time == scheduler::SCHED_EXIT)
      break;
    s.ctime = std::max (next_time, s.ctime + 1000L);
  }

  return EX_OK;
}

}
