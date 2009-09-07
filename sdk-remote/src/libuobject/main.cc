/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/// \file libuobject/main.cc

#include <libport/cstdio>
#include <libport/unistd.h>
#include <cerrno>

#include <iostream>
#include <list>
#include <sstream>

#include <libport/cli.hh>
#include <libport/containers.hh>
#include <libport/foreach.hh>
#include <libport/lexical-cast.hh>
#include <libport/package-info.hh>
#include <libport/program-name.hh>
#include <libport/sysexits.hh>

#include <urbi/package-info.hh>
#include <urbi/uexternal.hh>
#include <urbi/umain.hh>
#include <urbi/uobject.hh>
#include <urbi/usyncclient.hh>
#include <libuobject/remote-ucontext-impl.hh>
using libport::program_name;

namespace urbi
{
  static impl::RemoteUContextImpl* defaultContext;

  UCallbackAction
  debug(const UMessage& msg)
  {
    msg.client.printf("DEBUG: got a message: %s\n",
                      string_cast(msg).c_str());
    return URBI_CONTINUE;
  }

  UCallbackAction
  static
  endProgram(const UMessage& msg)
  {
    msg.client.printf("ERROR: got a disconnection message: %s\n",
                      string_cast(msg).c_str());
    exit(1);
    return URBI_CONTINUE; //stupid gcc
  }

  static
  void
  usage()
  {
    std::cout <<
      "usage:\n" << libport::program_name() << " [OPTION]...\n"
      "\n"
      "Options:\n"
      "  -b, --buffer SIZE     input buffer size"
		 << " [" << UAbstractClient::URBI_BUFLEN << "]\n"
      "  -h, --help            display this message and exit\n"
      "  -H, --host ADDR       server host name"
                << " [" << UClient::DEFAULT_HOST << "]\n"
      "      --server          put remote in server mode\n"
      "  -p, --port PORT       tcp port URBI will listen to"
		 << " [" << UAbstractClient::URBI_PORT << "]\n"
      "  -r, --port-file FILE  file containing the port to listen to\n"
      "  -v, --version         print version information and exit\n"
      "  -d, --disconnect      exit program if disconnected(defaults)\n"
      "  -s, --stay-alive      do not exit program if disconnected\n"
		 << libport::exit (EX_OK);
  }

  static
  void
  version()
  {
    std::cout << urbi::package_info() << std::endl
              << libport::exit (EX_OK);
  }

  typedef std::vector<std::string> files_type;
  int
  initialize(const std::string& host, int port, size_t buflen,
	     bool exitOnDisconnect, bool server, const files_type& files)
  {
    std::cerr << program_name()
	      << ": " << urbi::package_info() << std::endl
	      << program_name()
	      << ": Remote Component Running on "
	      << host << " " << port << std::endl;
              USyncClient::options o;
              o.server(server);
              new USyncClient(host, port, buflen, o);
    defaultContext = new impl::RemoteUContextImpl(
              dynamic_cast<USyncClient*>(getDefaultClient()));
    if (exitOnDisconnect)
    {
      if (!getDefaultClient() || getDefaultClient()->error())
	std::cerr << "ERROR: failed to connect, exiting..." << std::endl
		  << libport::exit(1);
      getDefaultClient()->setClientErrorCallback(callback(&endProgram));
    }
   if (!getDefaultClient() || getDefaultClient()->error())
      return 1;

#ifdef LIBURBIDEBUG
    getDefaultClient()->setWildcardCallback(callback(&debug));
#else
    getDefaultClient()->setErrorCallback(callback(&debug));
#endif

    // Wait for client to be connected if in server mode.
    while (getDefaultClient()
           && !getDefaultClient()->error()
           && !getDefaultClient()->init())
      usleep(20000);

    // Waiting for connectionID.
    while (getDefaultClient()
           && getDefaultClient()->connectionID() == "")
      usleep(5000);
    defaultContext->init();
   //baseURBIStarter::init();
    // Send a ';' since UObject likely sent a serie of piped commands.
    URBI_SEND_COMMAND("");
    // Load initialization files.
    foreach (const std::string& file, files)
      getDefaultClient()->sendFile(file);
    return 0;
  }

  namespace
  {
    static
    void
    argument_with_option(const char* longopt,
                         char shortopt,
                         const std::string& val)
    {
      std::cerr
        << program_name()
        << ": warning: arguments without options are deprecated"
        << std::endl
        << "use `-" << shortopt << ' ' << val << '\''
        << " or `--" << longopt << ' ' << val << "' instead"
        << std::endl
        << "Try `" << program_name() << " --help' for more information."
        << std::endl;
    }

  }


  int
  main(const libport::cli_args_type& args, bool block, bool)
  {
    std::string host = UClient::DEFAULT_HOST;
    bool exitOnDisconnect = true;
    int port = UAbstractClient::URBI_PORT;
    bool server = false;
    size_t buflen = UAbstractClient::URBI_BUFLEN;
    // Files to load
    files_type files;

    // The number of the next (non-option) argument.
    unsigned argp = 1;
    for (unsigned i = 1; i < args.size(); ++i)
    {
      const std::string& arg = args[i];
      if (arg == "--buffer" || arg == "-b")
	buflen = libport::convert_argument<size_t>(args, i++);
      else if (arg == "--disconnect" || arg == "-d")
	exitOnDisconnect = true;
      else if (arg == "--file" || arg == "-f")
        files.push_back(libport::convert_argument<const char*>(args, i++));
      else if (arg == "--stay-alive" || arg == "-s")
	exitOnDisconnect = false;
      else if (arg == "--help" || arg == "-h")
	usage();
      else if (arg == "--host" || arg == "-H")
	host = libport::convert_argument<std::string>(args, i++);
      else if (arg == "--port" || arg == "-p")
	port = libport::convert_argument<unsigned>(args, i++);
      else if (arg == "--port-file" || arg == "-r")
	port =
          (libport::file_contents_get<int>
           (libport::convert_argument<const char*>(args, i++)));
      else if (arg == "--server")
	server = true;
      else if (arg == "--version" || arg == "-v")
	version();
      else if (arg[0] == '-')
	libport::invalid_option(arg);
      else
	// A genuine argument.
	switch (argp++)
	{
	  case 1:
            argument_with_option("host", 'H', args[i]);
	    host = args[i];
	    break;
	  case 2:
            argument_with_option("port", 'p', args[i]);
	    port = libport::convert_argument<unsigned> ("port", args[i].c_str());
	    break;
	  default:
	    std::cerr << "unexpected argument: " << arg << std::endl
		      << libport::exit(EX_USAGE);
	}
    }

    initialize(host, port, buflen, exitOnDisconnect, server, files);

    if (block)
      while (true)
        usleep(30000000);
    return 0;
  }

}
