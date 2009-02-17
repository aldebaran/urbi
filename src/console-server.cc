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
#include <libport/sys/socket.h>
#include <libport/thread.hh>
#include <libport/utime.hh>

#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <libltdl/ltdl.h>

// Inclusion order matters for windows. Leave userver.hh after network.hh.
#include <network/bsdnet/network.hh>
#include <kernel/uqueue.hh>
#include <kernel/userver.hh>
#include <kernel/uconnection.hh>

#include <sched/configuration.hh>
#include <sched/scheduler.hh>

#include <kernel/ubanner.hh>
#include <object/system.hh>
#include <urbi/export.hh>
#include <urbi/umain.hh>
#include <urbi/uobject.hh>

using libport::program_name;

class ConsoleServer
  : public kernel::UServer
{
public:
  ConsoleServer(bool fast)
    : kernel::UServer("console"), fast(fast), ctime(0)
  {
  }

  virtual ~ConsoleServer()
  {}

  virtual void reset()
  {}

  virtual void reboot()
  {}

  virtual libport::utime_t getTime()
  {
    return fast ? ctime : libport::utime();
  }

  virtual
  UErrorValue
  save_file(const std::string& filename, const std::string& content)
  {
    //! \todo check this code
    std::ofstream os(filename.c_str());
    os << content;
    os.close();
    return os.good() ? USUCCESS : UFAIL;
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
  usage()
  {
    boost::format fmt(
      "usage: %s [OPTION].. PROGRAM_FILE ARGS...\n"
      "\n"
      "  PROGRAM_FILE   Urbi script to load.  `-' stands for standard input\n"
      "  ARGS           user arguments passed to PROGRAM_FILE\n"
      "\n"
      "Options:\n"
      "  -h, --help               display this message and exit successfully\n"
      "  -v, --version            display version information\n"
      "  -P, --period PERIOD      ignored for backward compatibility\n"
      "  -H, --host HOST          the address to listen on (default: all)\n"
      "  -p, --port PORT          TCP port to listen to\n"
      "  -n, --no-network         disable networking\n"
      "  -e, --expression SCRIPT  run SCRIPT\n"
      "  -F, --fast               ignore system time, go as fast as possible\n"
      "  -f, --file=FILE          load FILE\n"
      "  -i, --interactive        read and parse stdin in a nonblocking way\n"
      "  -s, --stack-size=SIZE    set the job stack size in KB\n"
      "  -w, --port-file FILE     write port number to specified file.\n");

    throw urbi::Exit(EX_OK, str(fmt % program_name()));
  }

  static
  void
  version()
  {
    std::stringstream dump;
    kernel::userver_package_info_dump(dump) << std::endl;
    throw urbi::Exit(EX_OK, dump.str());
  }
}

namespace urbi
{

  /// Command line options needed in the main loop.
  struct LoopData
  {
    LoopData()
      : interactive(false)
      , fast(false)
      , network(true)
      , server(0)
      {}

    bool interactive;
    bool fast;
    bool network;
    ConsoleServer* server;
  };

  int main_loop(LoopData& l);


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

  static std::string convert_input_file(const std::string& arg)
  {
    return (arg == "-") ? "/dev/stdin" : arg;
  }

  /// Data to send to the server.
  struct Input
  {
    Input(bool f, const std::string& v)
      : file_p(f), value(v)
    {}
    /// Whether its a file (or a litteral).
    bool file_p;
    std::string value;
  };

  URBI_SDK_API int
  main(const libport::cli_args_type& args, bool block, bool errors)
  {
    if (errors)
    {
      try
      {
        return main(args, block);
      }
      catch (const urbi::Exit& e)
      {
        std::cerr << e.what() << std::endl;
        return e.error_get();
      }
    }

    // Input files.
    typedef std::vector<Input> input_type;
    input_type input;
    /// The IP to bind. "" means every interface.
    std::string arg_host;
    /// The port to use.  0 means automatic selection.
    int arg_port = 0;
    /// Where to write the port we use.
    std::string arg_port_filename;
    /// The size of the stacks.
    size_t arg_stack_size = 0;
    /// The log verbosity level.
    unsigned arg_verbosity = 0;

    // Parse the command line.
    LoopData data;
    unsigned i;
    for (i = 1; i < args.size(); ++i)
    {
      const std::string& arg = args[i];

      if (arg == "--debug")
        arg_verbosity = libport::convert_argument<unsigned> (args, i++);
      else if (arg == "--fast" || arg == "-F")
        data.fast = true;
      else if (arg == "--expression" || arg == "-e")
        input.push_back(
          Input(false,
                libport::convert_argument<std::string>(args, i++)));
      else if (arg == "--file" || arg == "-f")
        input.push_back(
          Input(true,
                convert_input_file(
                  libport::convert_argument<std::string>(args, i++))));
      else if (arg == "--help" || arg == "-h")
        usage();
      else if (arg == "--host" || arg == "-H")
        arg_host = libport::convert_argument<std::string>(args, i++);
      else if (arg == "--interactive" || arg == "-i")
        data.interactive = true;
      else if (arg == "--no-network" || arg == "-n")
        data.network = false;
      else if (arg == "--period" || arg == "-P")
        (void) libport::convert_argument<int> (args, i++);
      else if (arg == "--port" || arg == "-p")
        arg_port = libport::convert_argument<int> (args, i++);
      else if (arg == "--port-file" || arg == "-w")
        arg_port_filename =
          libport::convert_argument<std::string>(args, i++);
      else if (arg == "--stack-size" || arg == "-s")
        arg_stack_size = libport::convert_argument<size_t> (args, i++);
      else if (arg == "--version" || arg == "-v")
        version();
      else if (arg[0] == '-' && arg[1] != 0)
        libport::invalid_option(arg);
      else
      {
        // Unrecognized option. This is a script file, followed by user args.
        object::system_set_program_name(arg);
        input.push_back(Input(true, convert_input_file(arg)));
        // Anything left is user argument
        for (++i; i < args.size(); ++i)
          object::system_push_argument(args[i]);
        break;
      }
    }

    // Libtool traces.
    lt_dladd_log_function((lt_dllog_function*) &ltdebug, (void*) arg_verbosity);

    // If not defined in command line, use the envvar.
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
      sched::configuration.default_stack_size = arg_stack_size;
    }

    data.server = new ConsoleServer(data.fast);
    ConsoleServer& s = *data.server;
    int port = 0;
    if (data.network
        && !(port = Network::createTCPServer(arg_port, arg_host)))
    {
      boost::format fmt("%s: cannot bind to port %s");
      std::string message = str(fmt % program_name() % arg_port);
      if (!arg_host.empty())
      {
        boost::format fmt(" on %s");
        message += str(fmt % arg_host);
      }
      throw urbi::Exit(EX_UNAVAILABLE, message);
    }

    s.initialize();

    // Write the port file after initialize returned; that is, after
    // urbi.u is loaded.
    if (!arg_port_filename.empty())
      std::ofstream(arg_port_filename.c_str(), std::ios::out)
        << port << std::endl;

    kernel::UConnection& c = s.ghost_connection_get();
#ifdef ENABLE_DEBUG_TRACES
    std::cerr << program_name()
              << ": got ghost connection" << std::endl;
#endif

    foreach (const Input& i, input)
    {
      int res = USUCCESS;
      if (i.file_p)
        res = s.load_file(i.value, c.recv_queue_get());
      else
        c.recv_queue_get().push(i.value.c_str());
      if (res != USUCCESS)
      {
        boost::format fmt("%s: failed to process %s");
        throw urbi::Exit(EX_NOINPUT, str(fmt % program_name() % i.value));
      }
    }

    c.new_data_added_get() = true;

#ifdef ENABLE_DEBUG_TRACES
    std::cerr << program_name() << ": going to work..." << std::endl;
#endif
    if (block)
      return main_loop(data);
    else
      libport::startThread(new boost::function0<void>(
                             boost::bind(&main_loop, data)));
    return 0;
  }

  int
  main_loop(LoopData& data)
  {
    ConsoleServer& s = *data.server;
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
          s.ghost_connection_get().received(input);
      }

      if (data.network)
      {
        libport::utime_t select_time = 0;
        if (!data.fast)
        {
          select_time = std::max(next_time - libport::utime(), select_time);
          if (data.interactive)
            select_time = std::min(100000LL, select_time);
        }
        Network::selectAndProcess(select_time);
      }

      next_time = s.work();
      if (next_time == sched::SCHED_EXIT)
        break;
      s.ctime = std::max(next_time, s.ctime + 1000L);
    }

    return EX_OK;
  }

}
