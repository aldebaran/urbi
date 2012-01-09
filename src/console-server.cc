/*
 * Copyright (C) 2006-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/config.h>

#include <libport/unistd.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <libport/cstring>

#include <boost/function.hpp>
#include <libport/bind.hh>

#include <libport/asio.hh>
#include <libport/cli.hh>
#include <libport/debug.hh>
#include <libport/exception.hh>
#include <libport/foreach.hh>
#include <libport/format.hh>
#include <libport/sys/utsname.h>

#ifndef NO_OPTION_PARSER
# include <libport/option-parser.hh>
# include <libport/input-arguments.hh>
# include <libport/tokenizer.hh>
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

#include <sched/coroutine-local-storage.hh>

// Inclusion order matters for windows. Leave userver.hh after network.hh.
#include <urbi/kernel/userver.hh>
#include <urbi/kernel/uconnection.hh>

#include <sched/configuration.hh>
#include <sched/scheduler.hh>

#include <kernel/connection.hh>
#include <urbi/object/symbols.hh>
#include <urbi/object/global.hh>
#include <urbi/object/object.hh>
#include <urbi/object/float.hh>

#include <object/system.hh>
#include <urbi/export.hh>
#include <urbi/package-info.hh>
#include <urbi/umain.hh>
#include <urbi/uobject.hh>

GD_CATEGORY(Urbi);

#define URBI_EXIT(Status, ...)                 \
  throw urbi::Exit(Status, libport::format (__VA_ARGS__))

class ConsoleServer
  : public kernel::UServer
  , public libport::Socket
{
public:
  ConsoleServer(bool fast, UrbiRoot& root)
    : kernel::UServer(root)
    , libport::Socket(kernel::UServer::get_io_service())
    , fast(fast)
    , ctime(0)
  {}

  virtual ~ConsoleServer()
  {}

  virtual void reset()
  {}

  virtual void reboot()
  {}

  virtual libport::utime_t getTime() const
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


static void onCloseStdin(kernel::UConnection& c)
{
  GD_INFO("stdin Closed.");
  c.close();
//  object::objects_type args;
  // kernel::urbiserver->schedule(urbi::object::global_class,
  //                             SYMBOL(shutdown), args);
}


namespace
{
#if !defined WIN32
  static
  void
  onReadStdin(boost::asio::posix::stream_descriptor& sd,
              boost::asio::streambuf& buffer,
              kernel::UConnection& c,
              const boost::system::error_code& erc,
              size_t len)
  {
    GD_INFO_DEBUG("onReadStdin");
    if (erc)
    {
      if (erc == boost::asio::error::eof)
        onCloseStdin(c);
      return;
    }
    std::string s;
    s.resize(len);
    std::istream is(&buffer);
    is.read(&s[0], len);
    c.received(s);
    GD_FINFO_DEBUG("onReadStdin: %s", s);
    boost::asio::async_read(sd, buffer, boost::asio::transfer_at_least(1),
                            boost::bind(&onReadStdin,
                                        boost::ref(sd),
                                        boost::ref(buffer),
                                        boost::ref(c),
                                        _1, _2));
  }
#endif

#ifndef NO_OPTION_PARSER
  static
  void
  help(libport::OptionParser& parser)
  {
    std::stringstream output;
    output
      << "usage: " << libport::program_name()
      << " [OPTIONS] [PROGRAM_FILE | -- ] [ARGS...]" << std::endl
      << std::endl
      << "  PROGRAM_FILE   Urbi script to load."
      << "  `-' stands for standard input" << std::endl
      << "  ARGS           user arguments passed to PROGRAM_FILE" << std::endl
      << parser;
    throw urbi::Exit(EX_OK, output.str());
  }

  static
  void
  version()
  {
    std::cout << urbi::package_info() << std::endl
              << libport::exit(EX_OK);
  }

  static
  void
  forbid_option(const std::string& arg)
  {
    if (arg.size() > 1 && arg[0] == '-')
      URBI_EXIT(EX_USAGE,
                "unrecognized command line option: %s", arg);
  }
#endif
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
  libport::Socket*
  connectionFactory()
  {
    kernel::Connection* res = new kernel::Connection();
    kernel::urbiserver->connection_add(res);
    return res;
  }

#ifndef NO_OPTION_PARSER
  /// Data to send to the server.
  struct DataSender : libport::opts::DataVisitor
  {
    typedef libport::opts::DataVisitor super_type;
    DataSender(kernel::UServer& server, kernel::UConnection& connection)
      : server_(server)
      , connection_(connection)
    {}

    using super_type::operator();

    void
    operator()(const libport::opts::TextData& d)
    {
      connection_.received(d.command_);
    }

    void
    operator()(const libport::opts::FileData& d)
    {
      if (server_.load_file(d.filename_, connection_)
          != USUCCESS)
        URBI_EXIT(EX_NOINPUT,
                  "failed to process file %s: %s",
                  d.filename_, strerror(errno));
    }

    kernel::UServer& server_;
    kernel::UConnection& connection_;
  };
#endif

  static
  int
  init(const libport::cli_args_type& _args, bool errors,
       libport::Semaphore* sem, UrbiRoot& urbi_root)
  {
    libport::Finally f;
    if (sem)
      f << boost::bind(&libport::Semaphore::operator++, sem);
    if (errors)
    {
      try
      {
        return init(_args, false, sem, urbi_root);
      }
      catch (const urbi::Exit& e)
      {
        std::cerr << libport::program_name() << ": " << e.what() << std::endl;
        return e.error_get();
      }
      catch (const std::exception& e)
      {
        std::cerr << libport::program_name() << ": " << e.what() << std::endl;
        return 1;
      }
    }


    libport::cli_args_type args = _args;

    object::system_set_program_name(args[0]);
    args.erase(args.begin());
#ifndef NO_OPTION_PARSER
    // Detect shebang mode
    if (!args.empty() && !args[0].empty() && args[0][1] == '-'
        && args[0].find_first_of(' ') != args[0].npos)
    {
      // All our arguments are in args[0]
      std::string arg0(args[0]);
      libport::cli_args_type nargs;
      foreach(const std::string& arg, libport::make_tokenizer(arg0, " "))
        nargs.push_back(arg);
      for (size_t i = 1; i< args.size(); ++i)
        nargs.push_back(args[i]);
      args = nargs;
    }
#endif
    /// The size of the stacks.
    size_t arg_stack_size = 0;

    // Parse the command line.
    LoopData data;
#ifndef NO_OPTION_PARSER
    libport::OptionFlag
      arg_fast("ignore system time, go as fast as possible", "fast", 'F'),
      arg_interactive("read stdin in a nonblocking way", "interactive", 'i'),
      arg_no_net("ignored for backward compatibility", "no-network", 'n'),
      arg_no_banner("do not send the banner to incoming clients", "quiet", 'q'),
      arg_root("output Urbi root and exit", "print-root");

    libport::OptionValue
      arg_stack("set the job stack size in KB", "stack-size", 's', "SIZE");

    libport::OptionsEnd arg_remaining(true);
    {
      libport::OptionParser parser;
      parser
        << "Options:"
        << libport::opts::help
        << libport::opts::version
        << arg_root
        << "Tuning:"
# ifndef LIBPORT_DEBUG_DISABLE
        << libport::opts::debug
# endif
        << arg_fast
        << arg_stack
        << arg_no_banner
        << "Networking:"
        << libport::opts::host_l
        << libport::opts::port_l
        << libport::opts::port_file_l
        << arg_no_net
        << "Execution:"
        << libport::opts::exp
        << libport::opts::file
        << libport::opts::module
        << arg_interactive
        << arg_remaining
        ;
      try
      {
        args = parser(args);
      }
      catch (libport::Error& e)
      {
        URBI_EXIT(EX_USAGE, "command line error: %s", e.what());
      }

      if (libport::opts::help.get())
        help(parser);
      if (libport::opts::version.get())
        version();
      if (arg_root.get())
        std::cout << urbi_root.root() << std::endl << libport::exit(0);

#endif
      data.interactive = IF_OPTION_PARSER(arg_interactive.get(),
                                          getenv("URBI_INTERACTIVE"));
#ifndef NO_OPTION_PARSER

      data.fast = arg_fast.get();

      arg_stack_size = arg_stack.get<size_t>(static_cast<size_t>(0));

     // Since arg_remaining ate everything, args should be empty
     // unless the user made a mistake.
     if (!args.empty())
       forbid_option(args[0]);

      // Unrecognized options: script files, followed by user args, or
      // '--' followed by user args.
      libport::OptionsEnd::values_type remaining_args = arg_remaining.get();
      if (!remaining_args.empty())
      {
        unsigned startPos = 0;
        if (!arg_remaining.found_separator())
        {
          // First argument is an input file.
          libport::opts::input_arguments.add_file(remaining_args[0]);
          ++startPos;
        }
        // Anything left is user argument
        for (unsigned i = startPos; i < remaining_args.size(); ++i)
        {
          std::string arg = remaining_args[i];
          object::system_push_argument(arg);
        }
      }
    }
#endif

    // If not defined in command line, use the envvar.
    if (IF_OPTION_PARSER(!arg_stack.filled() && , )  getenv("URBI_STACK_SIZE"))
      arg_stack_size = libport::convert_envvar<size_t> ("URBI_STACK_SIZE");

    if (arg_stack_size)
    {
      // Make sure the result is a multiple of the page size.  This
      // required at least on OSX (which unfortunately fails with
      // errno = 0).
      arg_stack_size *= 1024;
      size_t pagesize = getpagesize();
      arg_stack_size = ((arg_stack_size + pagesize - 1) / pagesize) * pagesize;
      sched::configuration.default_stack_size = arg_stack_size;
    }

    data.server = new ConsoleServer(data.fast, urbi_root);
    ConsoleServer& s = *data.server;

    /*----------.
    | --quiet.  |
    `----------*/

#ifndef NO_OPTION_PARSER
    if (arg_no_banner.get())
      s.opt_banner_set(false);
#endif

    /*----------------.
    | --port/--host.  |
    `----------------*/
    int port = -1;
    {
      int desired_port = IF_OPTION_PARSER(libport::opts::port_l.get<int>(-1),
                                          54000);
      if (desired_port != -1)
      {
        std::string host =
          IF_OPTION_PARSER(libport::opts::host_l.value("127.0.0.1"),"0.0.0.0");
        if (boost::system::error_code err =
            s.listen(&connectionFactory, host, desired_port))
          URBI_EXIT(EX_UNAVAILABLE,
                    "cannot listen to port %s:%s: %s",
                    host, desired_port, err.message());
        port = s.getLocalPort();
        // Port not allocated at all, or port differs from (non null)
        // request.
        if (!port
            || (desired_port && port != desired_port))
          URBI_EXIT(EX_UNAVAILABLE,
                    "cannot listen to port %s:%s", host, desired_port);
      }
    }
    data.network = 0 < port;

#ifndef NO_OPTION_PARSER
    // If neither -e, -f, or -P is used, enable -i.
    if (   ! data.network
        && ! libport::opts::input_arguments.has_exps()
        && ! libport::opts::input_arguments.has_files())
      data.interactive = true;
#endif

    // In Urbi: System.listenPort = <port>.
    object::system_class->slot_set(SYMBOL(listenPort),
                                   object::to_urbi(port),
                                   true);
    kernel::urbiserver->interactive_set(data.interactive);
    object::system_class->slot_set(SYMBOL(fast),
                                   object::to_urbi(data.fast),
                                   true);
    s.initialize(data.interactive);

    /*--------------.
    | --port-file.  |
    `--------------*/
    // Write the port file after initialize returned; that is, after
    // urbi.u is loaded.
    IF_OPTION_PARSER(
    if (libport::opts::port_file_l.filled())
      std::ofstream(libport::opts::port_file_l.value().c_str(), std::ios::out)
        << port << std::endl;,
    )

    kernel::UConnection& c = s.ghost_connection_get();
#if !defined WIN32
    libport::utsname machine;
    // The use of Boost::Asio to handle stdin/stdout does not work at
    // all on Leopard (major == 9).  On Snow Leopard (major == 10), it
    // requires some adjustments, see effectiveDisplay.
    if (data.interactive
        && (machine.system() != "Darwin"
            || 10 <= machine.release_major()))
    {
      boost::asio::posix::stream_descriptor* sd =
        new boost::asio::posix::stream_descriptor(
          s.kernel::UServer::get_io_service());
      sd->assign(0);
      boost::asio::streambuf* buffer = new boost::asio::streambuf;
      boost::asio::async_read(*sd, *buffer, boost::asio::transfer_at_least(1),
                              boost::bind(&onReadStdin,
                                          boost::ref(*sd),
                                          boost::ref(*buffer),
                                          boost::ref(c),
                                          _1, _2));
      // Under Mac OS X and linux, in interactive sessions, and unless there are
      // redirections, stdin, stdout, and stderr are bound together:
      // fcntl on one of them, affects the others.  Our using ASIO has
      // set stdin in non-blocking mode, thus stdout is in non-blocking
      // mode too.  This is not what we want.  So set it to blocking
      // all the time, except when we are calling System.poll().
      int flags = fcntl(STDOUT_FILENO, F_GETFL);
      ERRNO_RUN(fcntl, STDOUT_FILENO, F_SETFL, flags & ~O_NONBLOCK);
    }
#endif
    GD_INFO_TRACE("got ghost connection");

#ifndef NO_OPTION_PARSER
    DataSender send(s, c);
    send(libport::opts::input_arguments);
    libport::opts::input_arguments.clear();
#endif

    c.received("");
    GD_INFO_TRACE("going to work...");

    if (sem)
      (*sem)++;

    return main_loop(data);
  }

  int
  main_loop(LoopData& data)
  {
    ConsoleServer& s = *data.server;
    libport::utime_t next_time = 0;
    libport::utsname machine;
    GD_FINFO_DEBUG("machine: %s", machine);
#if defined WIN32 || defined __APPLE__
    bool needs_read_stdin =
      (machine.system() != "Darwin" || machine.release_major() < 10);
    GD_FINFO_DEBUG("needs_read_stdin: %s", needs_read_stdin);
#endif
    while (true)
    {
#if defined WIN32 || defined __APPLE__
      if (needs_read_stdin && data.interactive)
      {
        std::string input;
        try
        {
          input = libport::read_stdin();
        }
        catch (const libport::exception::Exception& e)
        {
          std::cerr << libport::program_name() << ": "
                    << e.what() << std::endl;
          data.interactive = false;
          onCloseStdin(s.ghost_connection_get());
        }
        GD_FINFO_DEBUG("Got input: %s", input);
        if (!input.empty())
          s.ghost_connection_get().received(input);
      }
#endif
      next_time = s.work();
      GD_FINFO_DEBUG("next_time: %s (is exit: %s)",
                     next_time,
                     next_time == sched::SCHED_EXIT);
      if (next_time == sched::SCHED_EXIT)
        break;
      s.ctime = std::max(next_time, s.ctime + 1000L);
    }

    return EX_OK;
  }

#ifndef LIBPORT_DEBUG_DISABLE

static libport::local_data&
debugger_data_thread_coro_local()
{
# if ! defined __UCLIBC__ && defined LIBPORT_SCHED_MULTITHREAD
  // Per thread per coro storage.
  // Use only one thread_local_storage key, implementation may limit
  // the number of keys, or not free them, and we allocate/free a lot of
  // coroutines.
  typedef sched::CoroutineLocalStorage<libport::local_data> clocal;
  typedef boost::thread_specific_ptr<clocal> coro_storage;

  static coro_storage cstorage;

  clocal* tstorage = cstorage.get();
  if (!tstorage)
  {
    tstorage = new clocal;
    cstorage.reset(tstorage);
  }
  return tstorage->get();
#else
  // Per-coro storage in main-thread, per-thread otherwise
  static pthread_t main_thread = pthread_self();
  static sched::CoroutineLocalStorage<libport::local_data> clocal;
  static boost::thread_specific_ptr<libport::local_data> tlocal;
  libport::local_data* res;
  if (pthread_self() == main_thread)
  {
    res = &clocal.get();
  }
  else
  {
    res = tlocal.get();
    if (!res)
    {
      res = new libport::local_data;
      tlocal.reset(res);
    }
  }
  return *res;
#endif
}
#endif
  int
  main(const libport::cli_args_type& args,
       UrbiRoot& urbi_root, bool block, bool errors)
  {
    // Safet to call debugger_data_thread_coro_local() to initialize its static
    // members before passing it to GD or it could deadlock.
    debugger_data_thread_coro_local();
    GD_INIT_DEBUG_PER(debugger_data_thread_coro_local);

    if (block)
      return init(args, errors, 0, urbi_root);
    else
    {
      // The semaphore must survive this block, as init will use it when
      // exiting.
      libport::Semaphore* s = new libport::Semaphore;
      libport::startThread(boost::bind(&init, boost::ref(args),
                                       errors, s, boost::ref(urbi_root)));
      (*s)--;
      return 0;
    }
  }

}
