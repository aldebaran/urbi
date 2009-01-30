/**
   Send a binary file to the URBI server, to be saved in a variable.
*/

#include <vector>
#include <sys/types.h>
#include <libport/cstdio>

#include <libport/cli.hh>
#include <libport/foreach.hh>
#include <libport/program-name.hh>
#include <libport/sys/stat.h>
#include <libport/sysexits.hh>
#include <libport/unistd.h>
#include <libport/windows.hh>

#include <urbi/package-info.hh>
#include <urbi/uclient.hh>

using libport::program_name;

namespace
{
  static
  void
  usage()
  {
    std::cout <<
      "usage: " << program_name << " [OPTION].. [VAR FILE HEADERS]...\n"
      "\n"
      "  VAR      variable name into which the value is stored\n"
      "  FILE     contents to store\n"
      "  HEADERS  associated headers\n"
      "\n"
      "For instance:\n"
      "  " << program_name << " sounds.hello hello.wav WAV\n"
      "  " << program_name << " \"var Global.hello hello.wav WAV\n"
      "\n"
      "Options:\n"
      "  -h, --help        display this message and exit\n"
      "  -v, --version     display version information and exit\n"
      "  -H, --host HOST   the host running the Urbi server [localhost]\n"
      "  -p, --port PORT   the Urbi server port ["
                << urbi::UClient::URBI_PORT << "]\n"
      ;
    exit (EX_OK);
  }

  static
  void
  version()
  {
    std::cout << urbi::package_info() << std::endl
              << libport::exit(EX_OK);
  }
}


static urbi::UCallbackAction
dump(const urbi::UMessage& msg)
{
  // FIXME: This is absolutely not completely migrated.
  // To be finished -- Akim.
  switch (msg.type)
  {
    case urbi::MESSAGE_DATA:
      std::cout << msg << std::endl;
      break;

    case urbi::MESSAGE_ERROR:
    case urbi::MESSAGE_SYSTEM:
      std::cerr << msg << std::endl;
      break;
  }
  return urbi::URBI_CONTINUE;
}

static urbi::UCallbackAction
error(const urbi::UMessage& msg)
{
  dump(msg);
  exit(0);
}


/// The tripples given by the user.
struct data_type
{
  data_type(const char* v, const char* f, const char* h)
    : variable(v), file(f), headers(h)
  {}
  const char* variable;
  const char* file;
  const char* headers;
};

int
main(int argc, char * argv[])
{
  program_name = argv[0];
  /// Server host name.
  std::string host = "localhost";
  /// Server port.
  int port = urbi::UClient::URBI_PORT;

  std::vector<data_type> data;

  // Parse the command line.
  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];

    if (arg == "--help" || arg == "-h")
      usage();
    else if (arg == "--host" || arg == "-H")
      host = argv[++i];
    else if (arg == "--port" || arg == "-p")
      port = libport::convert_argument<int> (arg, argv[++i]);
    else if (arg == "--version" || arg == "-v")
      version();
    else if (arg[0] == '-' && arg[1] != 0)
      libport::invalid_option (arg);
    else
    {
      if (argc < i + 3)
        libport::missing_argument("too few arguments");
      data.push_back(data_type(argv[i], argv[i + 1], argv[i + 2]));
      i += 3;
    }
  }

  urbi::UClient client(host, port);
  client.setWildcardCallback(callback(&dump));
  client.setClientErrorCallback(callback(&error));

  if (client.error())
    std::cerr << libport::program_name << ": client failed to set up"
	      << std::endl
              << libport::exit(1);

  foreach (data_type d, data)
  {
    FILE *f = libport::streq(d.file, "-") ? stdin : fopen(d.file, "r");
    if (!f)
      std::cerr << program_name << ": cannot open " << d.file
                << ": " << strerror(errno)
                << libport::exit(EX_NOINPUT);

    char* buffer = 0;
    int pos = 0;
    // Read the whole file in memory
    if (f != stdin)
    {
      struct stat st;
      stat(d.file, &st);
      buffer = static_cast<char *> (malloc (st.st_size));
      if (!buffer)
        std::cerr << program_name << ": memory exhausted" << std::endl
                  << libport::exit(EX_OSERR);
      while (true)
      {
	size_t r = fread(buffer + pos, 1, st.st_size-pos, f);
	if (!r)
	  break;
	pos +=r;
      }
      //std::cerr <<"read "<<pos<<" bytes from "<<argv[argp+1]<<std::endl;
    }
    else
    {
      size_t sz = 10000;
      buffer = static_cast<char *> (malloc (sz));
      while (true)
      {
	if (sz-pos < 500)
	{
	  sz += 10000;
	  buffer = static_cast<char *> (realloc (buffer,sz));
          if (!buffer)
            std::cerr << program_name << ": memory exhausted" << std::endl
                      << libport::exit(EX_OSERR);
	  size_t r = fread(buffer + pos, 1, sz-pos, f);
	  if (!r)
	    break;
	  pos += r;
	}
      }
    }
    client.sendBin(buffer, pos, "%s = BIN %d %s;", d.variable, pos, d.headers);
  }
  sleep(1);
}
