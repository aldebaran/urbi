/// \file libuobject/main.cc

#include <libport/cstdio>
#include <cerrno>

#include <iostream>
#include <list>
#include <sstream>

#include <libport/cli.hh>
#include <libport/containers.hh>
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

  // This part is specific for standalone linux objects
  // LIBURBI 'Module mode'

  UCallbackAction
  dispatcher(const UMessage& msg)
  {
    //check message type
    if (msg.type != MESSAGE_DATA || msg.value->type != DATA_LIST)
    {
      msg.client.printf("Component Error: "
			"unknown message content, type %d\n",
			(int) msg.type);
      return URBI_CONTINUE;
    }

    UList& array = *msg.value->list;

    if (array.size()<2)
    {
      msg.client.printf("Component Error: Invalid number "
			"of arguments in the server message: %d\n",
			array.size());
      return URBI_CONTINUE;
    }

    if (array[0].type != DATA_DOUBLE)
    {
      msg.client.printf("Component Error: "
			"unknown server message type %d\n",
			(int) array[0].type);
      return URBI_CONTINUE;
    }

    switch ((USystemExternalMessage)(int)array[0])
    {
      case UEM_ASSIGNVALUE:
      {
        if (array.size() != 3)
        {
          msg.client.printf("Component Error: Invalid number "
                            "of arguments in the server message: %d"
                            " (expected 3)\n",
                            array.size());
          return URBI_CONTINUE;
        }
        UVarTable::iterator varmapfind = varmap->find(array[1]);
        if (varmapfind != varmap->end())
          for (std::list<UVar*>::iterator it = varmapfind->second.begin();
               it != varmapfind->second.end();
               ++it)
            (*it)->__update(array[2]);

        UTable::iterator monitormapfind = monitormap->find(array[1]);
        if (monitormapfind != monitormap->end())
          for (std::list<UGenericCallback*>::iterator
                 cbit = monitormapfind->second.begin();
               cbit != monitormapfind->second.end();
               ++cbit)
          {
            // test of return value here
            UList u;
            u.array.push_back(new UValue());
            u[0].storage = (*cbit)->storage;
            (*cbit)->__evalcall(u);
          }
      }
      break;

      case UEM_EVALFUNCTION:
      {
	std::list<UGenericCallback*> tmpfun = (*functionmap)[array[1]];
	std::list<UGenericCallback*>::iterator tmpfunit = tmpfun.begin();
	array.setOffset(3);
	UValue retval = (*tmpfunit)->__evalcall(array);
	array.setOffset(0);
	if (retval.type == DATA_VOID)
	  URBI_SEND_COMMAND("var " << (std::string) array[2]);
	else
	  URBI_SEND_COMMAND("var " << (std::string) array[2] << "=" << retval);
      }
      break;

      case UEM_EMITEVENT:
      {
        if (eventmap->find(array[1]) != eventmap->end())
        {
          std::list<UGenericCallback*> tmpfun = (*eventmap)[array[1]];
          for (std::list<UGenericCallback*>::iterator i = tmpfun.begin();
               i != tmpfun.end();
               ++i)
          {
            array.setOffset(2);
            (*i)->__evalcall(array);
            array.setOffset(0);
          }
        }
      }
      break;

      case UEM_ENDEVENT:
      {
        if (eventendmap->find(array[1]) != eventendmap->end())
        {
          std::list<UGenericCallback*> tmpfun = (*eventendmap)[array[1]];
          for (std::list<UGenericCallback*>::iterator i = tmpfun.begin();
               i != tmpfun.end();
               ++i)
          {
            array.setOffset(2);
            (*i)->__evalcall(array);
            array.setOffset(0);
          }
        }
      }

      case UEM_NEW:
      {
        std::list<baseURBIStarter*>::iterator found = objectlist->end();
        for (std::list<baseURBIStarter*>::iterator i = objectlist->begin();
             i != objectlist->end();
             ++i)
          if ((*i)->name == (std::string)array[2])
          {
            if (found != objectlist->end())
              msg.client.printf("Double object definition %s\n",
                                (*i)->name.c_str());
            else
              found = i;
          }

        if (found == objectlist->end())
          msg.client.printf("Unknown object definition %s\n",
                            ((std::string) array[2]).c_str());
        else
          (*found)->copy((std::string) array[1]);

      }
      break;

      case UEM_DELETE:
      {
        std::list<baseURBIStarter*>::iterator found = objectlist->end();
        for (std::list<baseURBIStarter*>::iterator i = objectlist->begin();
             i != objectlist->end();
             ++i)
          if ((*i)->name == (std::string)array[1])
          {
            if (found != objectlist->end())
              msg.client.printf("Double object definition %s\n",
                                (*i)->name.c_str());
            else
              found = i;
          }

        if (found == objectlist->end())
          msg.client.printf("Unknown object definition %s\n",
                            ((std::string) array[1]).c_str());
        else
        {
          // remove the object from objectlist or terminate
          // the component if there is nothing left
          if (objectlist->size() == 1)
            exit(0);
          else
            // delete the object
            delete *found;
        }
      }
      break;

      default:
        msg.client.printf("Component Error: "
                          "unknown server message type number %d\n",
                          (int)array[0]);
    }
    return URBI_CONTINUE;
  }


  UCallbackAction
  debug(const UMessage& msg)
  {
    std::stringstream mesg;
    mesg << msg;
    msg.client.printf("DEBUG: got a message: %s\n",  mesg.str().c_str());
    return URBI_CONTINUE;
  }

  UCallbackAction
  static endProgram(const UMessage& msg)
  {
    std::stringstream mesg;
    mesg << msg;
    msg.client.printf("ERROR: got a disconnection message: %s\n",
                      mesg.str().c_str());
    exit(1);
    return URBI_CONTINUE; //stupid gcc
  }

  static
  void
  usage()
  {
    std::cout <<
      "usage:\n" << libport::program_name << " [OPTION]...\n"
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

  static int
  initialize(const std::string& host, int port, int buflen,
	     bool exitOnDisconnect, bool server)
  {
    std::cerr << program_name
	      << ": " << urbi::package_info() << std::endl
	      << program_name
	      << ": Remote Component Running on "
	      << host << " " << port << std::endl;

    new USyncClient(host, port, buflen, server);

    if (exitOnDisconnect)
    {
      if (!getDefaultClient() || getDefaultClient()->error())
	std::cerr << "ERROR: failed to connect, exiting..." << std::endl
		  << libport::exit(1);
      getDefaultClient()->setClientErrorCallback(callback (&endProgram));
    }
   if (!getDefaultClient() || getDefaultClient()->error())
      return 1;

#ifdef LIBURBIDEBUG
    getDefaultClient()->setWildcardCallback(callback (&debug));
#else
    getDefaultClient()->setErrorCallback(callback (&debug));
#endif

    getDefaultClient()->setCallback(&dispatcher,
				    externalModuleTag.c_str());

    dummyUObject = new UObject (0);
    for (UStartlist::iterator i = objectlist->begin();
	 i != objectlist->end();
	 ++i)
      (*i)->init((*i)->name);
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
        << program_name
        << ": warning: arguments without options are deprecated"
        << std::endl
        << "use `-" << shortopt << ' ' << val << '\''
        << " or `--" << longopt << ' ' << val << "' instead"
        << std::endl
        << "Try `" << program_name << " --help' for more information."
        << std::endl;
    }

  }


  int
  main(const libport::cli_args_type& args, bool block)
  {
    program_name = args[0];

    std::string host = "localhost";
    bool exitOnDisconnect = true;
    int port = UAbstractClient::URBI_PORT;
    bool server = false;
    int buflen = UAbstractClient::URBI_BUFLEN;

    // The number of the next (non-option) argument.
    unsigned argp = 1;
    for (unsigned i = 1; i < args.size(); ++i)
    {
      const std::string& arg = args[i];
      if (arg == "--buffer" || arg == "-b")
	buflen = libport::convert_argument<unsigned>(args, i++);
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
