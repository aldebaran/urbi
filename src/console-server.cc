//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include <libport/unistd.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <libport/cstring>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <libport/cli.hh>
#include <libport/debug.hh>
#include <libport/exception.hh>
#include <libport/foreach.hh>
#include <libport/format.hh>
#ifndef NO_OPTION_PARSER
# include <libport/option-parser.hh>
# define IF_OPTION_PARSER(a, b) a
#else
 #define IF_OPTION_PARSER(a, b) b
#endif
#include <libport/package-info.hh>
#include <libport/program-name.hh>
#include <libport/read-stdin.hh>
#include <libport/semaphore.hh>
#include <libport/sys/socket.h>
#include <libport/sysexits.hh>
#include <libport/thread.hh>
#include <libport/utime.hh>
#include <libport/windows.hh>

#include <libltdl/ltdl.h>

// Inclusion order matters for windows. Leave userver.hh after network.hh.
#include <network/bsdnet/network.hh>
#include <kernel/uqueue.hh>
#include <kernel/userver.hh>
#include <kernel/uconnection.hh>

#include <sched/configuration.hh>
#include <sched/scheduler.hh>

#include <kernel/ubanner.hh>
#include <object/symbols.hh>
#include <object/object.hh>
#include <object/float.hh>
#include <object/system.hh>
#include <urbi/export.hh>
#include <urbi/umain.hh>
#include <urbi/uobject.hh>

GD_INIT();
GD_ADD_CATEGORY(URBI);

using libport::program_name;

class ConsoleServer
  : public kernel::UServer
{
public:
  ConsoleServer(bool fast)
    : kernel::UServer("console"), fast(fast), ctime(0)
  {}

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
#ifndef NO_OPTION_PARSER
  static
  void
  help(libport::OptionParser& parser)
  {
    std::stringstream output;
    output
      << "usage: " << libport::program_name()
      << " [OPTIONS] [PROGRAM_FILE] [ARGS...]" << std::endl
      << std::endl
      << "  PROGRAM_FILE   Urbi script to load."
      << "  `-' stands for standard input" << std::endl
      << "  ARGS           user arguments passed to PROGRAM_FILE" << std::endl
      << parser;
    throw urbi::Exit(EX_OK, output.str());
  }
#endif
  static
  void
  version()
  {
    throw urbi::Exit(EX_OK, kernel::UServer::package_info().signature());
  }

  static
  void
  forbid_option(const std::string& arg)
  {
    if (arg.size() > 1 && arg[0] == '-')
    {
      boost::format fmt("%s: unrecognized command line option: %s");
      throw urbi::Exit(EX_USAGE, str(fmt % libport::program_name() % arg));
    }
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
    /// Whether network connections are enabled.
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
    /// Whether it's a file (or a litteral).
    bool file_p;
    /// File name or litteral value.
    std::string value;
  };

  static int
  init(const libport::cli_args_type& _args, bool errors,
             libport::Semaphore* sem)
  {
    libport::Finally f;
    if (sem)
      f << boost::bind(&libport::Semaphore::operator++, sem);
    if (errors)
    {
      try
      {
        return init(_args, false, sem);
      }
      catch (const urbi::Exit& e)
      {
        std::cerr << program_name() << ": " << e.what() << std::endl;
        return e.error_get();
      }
    }

    GD_CATEGORY(URBI);

    libport::cli_args_type args = _args;

    object::system_set_program_name(args[0]);
    args.erase(args.begin());

    // Input files.
    typedef std::vector<Input> input_type;
    input_type input;
    /// The size of the stacks.
    size_t arg_stack_size = 0;

    // Parse the command line.
    LoopData data;
#ifndef NO_OPTION_PARSER
    libport::OptionFlag
      arg_fast("ignore system time, go as fast as possible",
               "fast", 'F'),
      arg_interactive("read and parse stdin in a nonblocking way",
                      "interactive", 'i'),
      arg_no_net("ignored for backward compatibility", "no-network", 'n');

    libport::OptionValue
      arg_dbg      ("", "debug"),
      arg_period   ("ignored for backward compatibility", "period", 'P'),
      arg_port_file("write port number to the specified file.",
                    "port-file", 'w', "FILE"),
      arg_stack    ("set the job stack size in KB", "stack-size", 's', "SIZE");

    libport::OptionValues
      arg_exps("run expression", "expression", 'e', "EXP");

    {
      libport::OptionParser parser;
      parser
        << "Options:"
        << arg_dbg
        << arg_fast
        << libport::opts::help
        << libport::opts::version
        << arg_period
        << "Tuning:"
        << arg_stack
        << "Networking:"
        << libport::opts::host_l
        << libport::opts::port_l
        << arg_port_file
        << arg_no_net
        << "Execution:"
        << arg_exps
        << libport::opts::files
        << arg_interactive
        ;
      try
      {
        args = parser(args);
      }
      catch (libport::Error& e)
      {
        boost::format fmt("%s: command line error: %s");
        throw Exit(EX_USAGE, str(fmt % libport::program_name() % e.what()));
      }

      if (libport::opts::help.get())
        help(parser);
      if (libport::opts::version.get())
        version();
#endif
      data.interactive = IF_OPTION_PARSER(arg_interactive.get(), true);
      data.fast = IF_OPTION_PARSER(arg_fast.get(), false);

#ifndef NO_OPTION_PARSER
      foreach (const std::string& exp, arg_exps.get())
        input.push_back(Input(false, exp));
      foreach (const std::string& file, libport::opts::files.get())
        input.push_back(Input(true, convert_input_file(file)));
      arg_stack_size = arg_stack.get<size_t>(static_cast<size_t>(0));

      // Unrecognized options. These are a script file, followed by user args.
      if (!args.empty())
      {
        forbid_option(args[0]);
        input.push_back(Input(true, convert_input_file(args[0])));
        // Anything left is user argument
        for (unsigned i = 1; i < args.size(); ++i)
        {
          std::string arg = args[i];
          forbid_option(arg);
          object::system_push_argument(arg);
        }
      }
    }
#endif

    // Libtool traces.
    lt_dladd_log_function((lt_dllog_function*) &ltdebug,
                          (void*) IF_OPTION_PARSER(arg_dbg.get<int>(0),0));

    // If not defined in command line, use the envvar.
    if (IF_OPTION_PARSER(!arg_stack.filled() && , )  getenv("URBI_STACK_SIZE"))
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

    /*----------------.
    | --port/--host.  |
    `----------------*/
    int port = -1;
    {
      int desired_port = IF_OPTION_PARSER(libport::opts::port_l.get<int>(-1),
                                          54000);
      if (desired_port != -1)
      {
        std::string host = IF_OPTION_PARSER(libport::opts::host_l.value(""),"");
        port = Network::createTCPServer(host, desired_port);
        if (!port)
          throw urbi::Exit
            (EX_UNAVAILABLE,
             libport::format("%s: cannot bind to port %s:%s",
                             program_name(), host, desired_port));
      }
    }
    data.network = 0 < port;
    // In Urbi: System.listenPort = <port>.
    object::system_class->slot_set(SYMBOL(listenPort),
                                   object::to_urbi(port),
                                   true);

    s.initialize(data.interactive);

    /*--------------.
    | --port-file.  |
    `--------------*/
    // Write the port file after initialize returned; that is, after
    // urbi.u is loaded.
    IF_OPTION_PARSER(
    if (arg_port_file.filled())
      std::ofstream(arg_port_file.value().c_str(), std::ios::out)
        << port << std::endl;,
    )

    kernel::UConnection& c = s.ghost_connection_get();
    GD_INFO("got ghost connection");

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

    GD_INFO("going to work...");

    if (sem)
      (*sem)++;

    return main_loop(data);

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
          std::cerr << program_name() << ": "  << e.what() << std::endl;
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

  int
  main(const libport::cli_args_type& _args, bool block, bool errors)
  {
    if (block)
      return init(_args, errors, 0);
    else
    {
      // The semaphore must survive this block, as init will use it when
      // exiting.
      libport::Semaphore* s = new libport::Semaphore;
      libport::startThread(boost::bind(&init, boost::ref(_args),
                                       errors, s));
      (*s)--;
      return 0;
    }
  }

}
