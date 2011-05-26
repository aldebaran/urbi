/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuobject/main.cc

#include <libport/cerrno>
#include <libport/cstdio>
#include <libport/debug.hh>
#include <libport/unistd.h>

#include <iostream>
#include <list>
#include <sstream>

#include <libport/debug.hh>
#include <libport/cli.hh>
#include <libport/containers.hh>
#include <libport/foreach.hh>
#include <libport/input-arguments.hh>
#include <libport/lexical-cast.hh>
#include <libport/package-info.hh>
#include <libport/program-name.hh>
#include <libport/sysexits.hh>
#include <libport/xltdl.hh>

#include <urbi/package-info.hh>
#include <urbi/uexternal.hh>
#include <urbi/umain.hh>
#include <urbi/umessage.hh>
#include <urbi/uobject.hh>
#include <urbi/urbi-root.hh>
#include <urbi/usyncclient.hh>
#include <libuobject/remote-ucontext-impl.hh>
using libport::program_name;

GD_CATEGORY(Urbi.UrbiLaunch);

namespace urbi
{
  static impl::RemoteUContextImpl* defaultContext;

  static
  UCallbackAction
  debug(const UMessage& msg)
  {
    LIBPORT_USE(msg);
    GD_SWARN("unexpected message: " << msg);
    return URBI_CONTINUE;
  }

  ATTRIBUTE_NORETURN
  static
  UCallbackAction
  endProgram(const UMessage& msg)
  {
    LIBPORT_USE(msg);
    GD_SWARN("got a disconnection message: " << msg);
    exit(0);
  }

  ATTRIBUTE_NORETURN
  static
  void
  help(libport::OptionParser& parser)
  {
    std::cout
      << "usage: " << libport::program_name() << " [OPTION]..." << std::endl
      << parser
      << libport::exit(EX_OK);
  }

  ATTRIBUTE_NORETURN
  static
  void
  version()
  {
    std::cout << urbi::package_info() << std::endl
              << libport::exit(EX_OK);
  }


  typedef std::vector<std::string> strings_type;
  static
  void
  load_modules(UrbiRoot& urbi_root, const strings_type& modules)
  {
    // If URBI_UOBJECT_PATH is not defined, first look in ., then in the
    // stdlib.
    std::string uobject_path = libport::xgetenv("URBI_UOBJECT_PATH", ".:");
    // Load the modules using our uobject library path.
    libport::xlt_advise dl;
    dl.ext().path().push_back(uobject_path, ":");
    foreach(const std::string& s, urbi_root.uobjects_path())
      dl.path().push_back(s);
    foreach (const std::string& m, modules)
      dl.open(m).detach();
  }


  /// Data to send to the server.
  struct DataSender : libport::opts::DataVisitor
  {
    typedef libport::opts::DataVisitor super_type;
    DataSender(UClient& client)
      : client_(client)
    {}

    using super_type::operator();

    void
    operator()(const libport::opts::TextData& d)
    {
      client_.send(d.command_);
    }

    void
    operator()(const libport::opts::FileData& d)
    {
      client_.sendFile(d.filename_);
    }

    UClient& client_;
  };

  ATTRIBUTE_NORETURN
  static
  void
  describe(std::ostream& o)
  {
    foreach (baseURBIStarter* s, baseURBIStarter::list())
      o << s->name << std::endl;
    foreach (baseURBIStarterHub* s, baseURBIStarterHub::list())
      o << s->name << std::endl;
    exit(0);
  }

  static
  int
  initialize(const std::string& host, int port, size_t buflen,
	     bool exitOnDisconnect, bool server,
             bool useSyncClient)
  {
    GD_FINFO_TRACE("this is %s", program_name());
    GD_SINFO_TRACE(urbi::package_info());
    GD_FINFO_TRACE("remote component running on %s:%s", host, port);
    UClient* client;
    if (useSyncClient)
    {
      USyncClient::options o;
      o.server(server);
      client = new USyncClient(host, port, buflen, o);
    }
    else
    {
      GD_WARN("the no-sync-client mode is dangerous.  "
              "Any attempt to use synchronous operation will crash"
              " your program.");
      UClient::options o;
      o.server(server);
      client = new UClient(host, port, buflen, o);
    }

    if (!client || client->error())
      std::cerr << "ERROR: failed to connect, exiting..." << std::endl
                << libport::exit(1);

    setDefaultClient(client);
    if (exitOnDisconnect)
      client->setClientErrorCallback(callback(&endProgram));

#ifdef LIBURBIDEBUG
    client->setWildcardCallback(callback(&debug));
#else
    client->setErrorCallback(callback(&debug));
#endif

    // Wait for client to be connected if in server mode.
    // Also wait for initialization exchanges.
    while (!client->isConnected() && !client->error()
	   || client->connectionID().empty())
      usleep(20000);

    defaultContext = new impl::RemoteUContextImpl(
      (USyncClient*)dynamic_cast<UClient*>(getDefaultClient()));

    // Initialize in the correct thread.
    client->notifyCallbacks
      (UMessage(*getDefaultClient(), 0, externalModuleTag,
                libport::format("[%s]", UEM_INIT)));

    // Load files and expressions.
    DataSender send(*client);
    send(libport::opts::input_arguments);
    libport::opts::input_arguments.clear();
    return 0;
  }


  URBI_SDK_API int
  main(const libport::cli_args_type& args,
       UrbiRoot& urbi_root, bool block, bool)
  {
    GD_INIT();

    libport::OptionFlag
      arg_describe("describe loaded UObjects and exit", "describe"),
      arg_disconnect("exit program if disconnected (default)",
                     "disconnect", 'd'),
      arg_stay_alive("do not exit program if disconnected",
                     "stay-alive", 's'),
      arg_async("Use UClient instead of USyncClient",
                "no-sync-client"),
      arg_server("put remote in server mode", "server");
    libport::OptionValue
      arg_buffer    ("input buffer size",
                     "buffer", 'b', "SIZE"),
      arg_describe_file("describe loaded UObjects to FILE and exit",
                        "describe-file", 0, "FILE");
    libport::OptionValues
      arg_module    ("load the MODULE shared library",
                     "module", 'm', "MODULE");

    libport::OptionParser opt_parser;
    opt_parser
      << "UObject options:"
      << libport::opts::help
      << libport::opts::version
      << arg_describe
      << arg_describe_file
      << arg_async
      << arg_stay_alive
      << arg_disconnect
      << arg_server
      << "Network:"
      << libport::opts::host
      << libport::opts::port
      << libport::opts::port_file
      << arg_buffer
      << "Input:"
      << libport::opts::exp
      << libport::opts::file
      << arg_module;

    // The list of modules.
    libport::cli_args_type modules;
    try
    {
      modules = opt_parser(args);
    }
    catch (const libport::Error& e)
    {
      foreach (std::string wrong_arg, e.errors())
        libport::invalid_option(wrong_arg);
    }

    if (libport::opts::version.get())
      version();
    if (libport::opts::help.get())
      help(opt_parser);

    // Load the modules.
    load_modules(urbi_root, arg_module.get());

    if (arg_describe.get())
      describe(std::cout);
    if (arg_describe_file.filled())
    {
      std::ofstream o(arg_describe_file.value().c_str());
      describe(o);
    }

    std::string host = libport::opts::host.value(UClient::default_host());
    int port =
      (libport::opts::port_file.filled()
       ? libport::file_contents_get<int>(libport::opts::port_file.value())
       : libport::opts::port.get<int>(urbi::UClient::URBI_PORT));

    bool exitOnDisconnect = !arg_stay_alive.get() || arg_disconnect.get();
    bool server = arg_server.get();
    bool useSyncClient = !arg_async.get();
    size_t buflen = arg_buffer.get<size_t>(UAbstractClient::URBI_BUFLEN);

    initialize(host, port, buflen, exitOnDisconnect, server,
               useSyncClient);

    if (block)
      while (true)
        usleep(30000000);
    return 0;
  }

}
