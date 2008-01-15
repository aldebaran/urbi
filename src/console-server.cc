//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include "libport/unistd.h"
#include "libport/sysexits.hh"
#include "libport/windows.hh"
#include <fstream>

#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include "libport/cli.hh"
#include "libport/compiler.hh"
#include "libport/program-name.hh"
#include "libport/sysexits.hh"
#include "libport/utime.hh"
#include "libport/windows.hh"

// Inclusion order matters for windows. Leave userver.hh after network.hh.
#include <network/bsdnet/network.hh>
#include "kernel/userver.hh"
#include "kernel/uconnection.hh"

#include "console-server.hh"
#include "ubanner.hh"

namespace urbi
{
  ConsoleServer::ConsoleServer (const options::ConsoleServerOptions& opt)
    : UServer(opt.getPeriod (), 1.0, 64000000, "console"),
      options_ (opt),
      fast_ (opt.isFastModeEnabled ()),
      ctime_ (0)
  {
    BOOST_FOREACH (const std::string& s, opt.getPaths ())
      path.push_back (s);

    initNetwork ();
    initialize ();
    initGhostConnection ();
  }

  ConsoleServer::~ConsoleServer ()
  {
  }

  void
  ConsoleServer::initNetwork ()
  {
    if (!Network::createTCPServer(options_.getPort (),
				  options_.getBindedAddress ().c_str ()))
      options_.error ((boost::format ("cannot bind to port %1% on %2%")
		       % options_.getPort ()
		       % options_.getBindedAddress ()).str (), EX_UNAVAILABLE);

    Network::startNetworkProcessingThread ();
  }

  void
  ConsoleServer::initGhostConnection ()
  {
    UConnection& c = getGhostConnection ();
    DEBUG(("Got ghost connection\n"));

    const std::string& in = options_.getInputFile ();
    if (in != "" &&
	loadFile(in.c_str (), &c.recvQueue ()) != USUCCESS)
      options_.error ((boost::format ("failed to process %1%") % in).str ());
    c.newDataAdded = true;
  }


  void
  ConsoleServer::shutdown ()
  {
    UServer::shutdown ();
    exit (EX_OK);
  }

  void
  ConsoleServer::beforeWork ()
  {
    ctime_ += static_cast<long long> (period_get ()) * 1000LL;
  }

  void
  ConsoleServer::reset ()
  {
  }

  void
  ConsoleServer::reboot ()
  {
  }

  ufloat
  ConsoleServer::getTime ()
  {
    if (fast_)
      return ctime_ / 1000LL;
    else
      return static_cast<ufloat> (libport::utime () / 1000LL);
  }

  ufloat
  ConsoleServer::getPower ()
  {
    return ufloat (1);
  }

  void
  ConsoleServer::getCustomHeader (int line, char* header, int maxlength)
  {
    // FIXME: This interface is really really ridiculous and fragile.
    strncpy(header, uconsole_banner[line], maxlength);
  }

  UErrorValue
  ConsoleServer::saveFile (const char* filename, const char* content)
  {
    //! \todo check this code
    std::ofstream os (filename);
    os << content;
    os.close ();
    return os.good () ? USUCCESS : UFAIL;
  }

  void
  ConsoleServer::effectiveDisplay (const char* t)
  {
    std::cout << t;
  }
}

int
main (int argc, const char* argv[])
{
  // Parse options.
  options::ConsoleServerOptions options (argc, argv);
  options.processArguments ();

  urbi::ConsoleServer s (options);

  std::cerr << libport::program_name << ": going to work..." << std::endl;
  if (options.isFastModeEnabled ())
    while(true)
      s.work();
  else
    while (true)
    {
      long long startTime = libport::utime();
      ufloat period = s.period_get() * 1000;
      while (libport::utime() < startTime + period)
	usleep (1);
      s.work ();
    }
}
