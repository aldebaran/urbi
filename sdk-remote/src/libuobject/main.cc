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

using libport::program_name;

namespace urbi
{

  namespace
  {

    /// Find in baseURBIStarter::list an object named \a named.
    /// Report errors to \a client.
    /// \return a pointer to the object found, 0 otherwise.
    static
    baseURBIStarter*
    find_object(UAbstractClient& client,
                const std::string& name)
    {
      typedef baseURBIStarter::list_type::iterator iterator;
      iterator i_end = baseURBIStarter::list().end();
      iterator found = i_end;
      for (iterator i = baseURBIStarter::list().begin(); i != i_end; ++i)
        if ((*i)->name == name)
        {
          if (found != i_end)
            client.printf("Double object definition %s\n", name.c_str());
          else
            found = i;
        }

      if (found == i_end)
      {
        client.printf("Unknown object definition %s\n", name.c_str());
        return 0;
      }
      else
        return *found;
    }

    /// Look for the function args[1] in \a t, and make a call to the
    /// associated callback with arguments (args[2], args[3], etc.).
    static
    void
    eval_call(UTable& t, UList& args)
    {
      if (UTable::callbacks_type* cs = t.find0(args[1]))
      {
        args.setOffset(2);
        foreach (UGenericCallback* c, *cs)
          c->__evalcall(args);
        args.setOffset(0);
      }
    }

  }

  UCallbackAction
  dispatcher(const UMessage& msg)
  {
    typedef UTable::callbacks_type callbacks_type;
    //check message type
    if (msg.type != MESSAGE_DATA || msg.value->type != DATA_LIST)
    {
      msg.client.printf("Component Error: "
			"unknown message content, type %d\n",
			msg.type);
      return URBI_CONTINUE;
    }

    UList& array = *msg.value->list;

    if (array.size() < 2)
    {
      msg.client.printf("Component Error: Invalid number "
			"of arguments in the server message: %lu\n",
			static_cast<unsigned long>(array.size()));
      return URBI_CONTINUE;
    }

    if (array[0].type != DATA_DOUBLE)
    {
      msg.client.printf("Component Error: "
			"invalid server message type %d\n",
			array[0].type);
      return URBI_CONTINUE;
    }

    switch ((USystemExternalMessage)(int)array[0])
    {
      case UEM_ASSIGNVALUE:
      {
        if (array.size() != 3)
        {
          msg.client.printf("Component Error: Invalid number "
                            "of arguments in the server message: %lu"
                            " (expected 3)\n",
                            static_cast<unsigned long>(array.size()));
          return URBI_CONTINUE;
        }
        UVarTable::iterator varmapfind = varmap().find(array[1]);
        if (varmapfind != varmap().end())
          for (std::list<UVar*>::iterator it = varmapfind->second.begin();
               it != varmapfind->second.end();
               ++it)
            (*it)->__update(array[2]);

        if (callbacks_type* cs = monitormap().find0(array[1]))
          for (callbacks_type::iterator i = cs->begin();
               i != cs->end(); ++i)
          {
            // test of return value here
            UList u;
            u.array.push_back(new UValue());
            u[0].storage = (*i)->storage;
            (*i)->__evalcall(u);
          }
      }
      break;

      case UEM_EVALFUNCTION:
      {
	callbacks_type tmpfun = functionmap()[array[1]];
        const std::string var = array[2];
	callbacks_type::iterator tmpfunit = tmpfun.begin();
	array.setOffset(3);
	UValue retval = (*tmpfunit)->__evalcall(array);
	array.setOffset(0);
	switch (retval.type)
        {
        case DATA_BINARY:
	  // Send it
	  if (UClient* client = urbi::getDefaultClient())
          {
            // URBI_SEND_COMMAND does not now how to send binary since it
            // depends on the kernel version.
            client->startPack();
            *client << " var  " << var << "=";
            client->send(retval);
            *client << ";";
            client->endPack();
          }
	  else
            // Send void if no client. Would block anyway.
	    goto case_void;
          break;

        case DATA_VOID:
        case_void:
          URBI_SEND_COMMAND("var " << var);
          break;

        default:
          URBI_SEND_COMMAND("var " << var << "=" << retval);
          break;
        }
      }
      break;

      case UEM_EMITEVENT:
        eval_call(eventmap(), array);
      break;

      case UEM_ENDEVENT:
        eval_call(eventendmap(), array);
      break;

      case UEM_NEW:
        if (baseURBIStarter* o = find_object(msg.client,
                                             (std::string)array[2]))
          o->copy((std::string) array[1]);
      break;

      case UEM_DELETE:
        if (baseURBIStarter* o = find_object(msg.client,
                                             (std::string)array[1]))
        {
          // remove the object from objectlist or terminate
          // the component if there is nothing left
          if (baseURBIStarter::list().size() == 1)
            exit(0);
          else
            // delete the object
            delete o;
        }
        break;

      default:
        msg.client.printf("Component Error: "
                          "unknown server message type number %d\n",
                          (int)array[0]);
        return URBI_CONTINUE;
    }
    // Send a terminating ';' since code send by the UObject API uses '|'.
    URBI_SEND_COMMAND("");
    return URBI_CONTINUE;
  }


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
      "  -H, --host ADDR       server host name   [localhost]\n"
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

  int
  initialize(const std::string& host, int port, size_t buflen,
	     bool exitOnDisconnect, bool server)
  {
    std::cerr << program_name()
	      << ": " << urbi::package_info() << std::endl
	      << program_name()
	      << ": Remote Component Running on "
	      << host << " " << port << std::endl;

    new USyncClient(host, port, buflen, server);

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

    getDefaultClient()->setCallback(&dispatcher,
				    externalModuleTag.c_str());

    // Wait for client to be connected if in server mode.
    while (getDefaultClient()
           && !getDefaultClient()->error()
           && !getDefaultClient()->init())
      usleep(20000);

    // Waiting for connectionID.
    while (getDefaultClient()
           && getDefaultClient()->connectionID() == "")
      usleep(5000);

    dummyUObject = new UObject(0);
    baseURBIStarter::init();
    // Send a ';' since UObject likely sent a serie of piped commands.
    URBI_SEND_COMMAND("");
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
    std::string host = "localhost";
    bool exitOnDisconnect = true;
    int port = UAbstractClient::URBI_PORT;
    bool server = false;
    size_t buflen = UAbstractClient::URBI_BUFLEN;

    // The number of the next (non-option) argument.
    unsigned argp = 1;
    for (unsigned i = 1; i < args.size(); ++i)
    {
      const std::string& arg = args[i];
      if (arg == "--buffer" || arg == "-b")
	buflen = libport::convert_argument<size_t>(args, i++);
      else if (arg == "--disconnect" || arg == "-d")
	exitOnDisconnect = true;
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

   initialize(host, port, buflen, exitOnDisconnect, server);

   if (block)
     while (true)
       usleep(30000000);
    return 0;
  }

}
