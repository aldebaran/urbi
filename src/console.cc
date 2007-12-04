//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include "config.h"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include "libport/windows.hh"
#include <iostream>
#include <fstream>

#include "libport/utime.hh"

#include "kernel/userver.hh"
#include "kernel/uconnection.hh"

#include "ubanner.hh"

class ConsoleServer
  : public UServer
{
public:
  ConsoleServer(int freq)
    : UServer(freq, "console")
  {
    if (const char* cp = getenv ("URBI_PATH"))
      // FIXME: Is there anything more elegant?
    {
      for (const char* end = strchr (cp, ':');
	   end;
	   cp = end + 1, end = strchr(cp, ':'))
	path.push_back(std::string(cp, end - cp));
      path.push_back(std::string(cp));
    }
  }

  virtual ~ConsoleServer()
  {}

  virtual void shutdown()
  {
    UServer::shutdown ();
    exit (0);
  }

  virtual void reset()
  {}

  virtual void reboot()
  {}

  virtual ufloat getTime()
  {
    return static_cast<ufloat>(libport::utime() / 1000LL);
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
  saveFile (const std::string& filename, const std::string& content)
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
};

namespace
{
  static
  void
  usage (const char* program_name, int estatus)
  {
    std::cout <<
      "usage: " << program_name << "[OPTIONS] [FILE]\n"
      "\n"
      "  FILE    to load\n"
      "\n"
      "Options:\n"
      "  -h, --help     display this message and exit successfully\n"
      "  -v, --version  display version information\n";
    exit (estatus);
  }

  static
  void
  version ()
  {
    userver_package_info_dump(std::cout) << std::endl;
    exit (0);
  }
}

int
main (int argc, const char* argv[])
{
  // Input file.
  const char *in = "/dev/stdin";
  // Parse the command line.
  {
    int argp = 1;
    for (int i = 1; i < argc; ++i)
    {
      std::string arg = argv[i];

      if (arg == "-h" || arg == "--help")
	usage(argv[0], 0);
      else if (arg == "-v" || arg == "--version")
	version();
      else
	// An argument.
	switch (argp++)
	{
	  case 1:
	    in = argv[i];
	    break;
	  default:
	    std::cerr << "Unexpected argument: " << arg << std::endl;
	    exit (1);
	    break;
	}
    }
  }

  ConsoleServer s (10);

  s.initialize ();
  UConnection& c = s.getGhostConnection ();
  DEBUG(("Got ghost connection\n"));

  if (s.loadFile(in, &c.recvQueue ()) != USUCCESS)
  {
    std::cerr << argv[0] << ": failed to process " << in << std::endl;
    return 1;
  }

  c.newDataAdded = true;

  long long startTime = libport::utime();

  DEBUG(("Going to work...\n"));
  while (true)
  {
    ufloat freq = s.getFrequency() * 1000;
    while (libport::utime() < (startTime + freq))
      usleep (1);
    s.work ();
  }
}
