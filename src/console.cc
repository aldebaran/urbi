#include "config.h"
#include "version.hh"

#include <fstream>

#include "libport/utime.hh"

#include "userver.hh"
#include "ughostconnection.hh"

#define URBI_BUFSIZ 1024

class ConsoleServer
  : public UServer
{
public:
  ConsoleServer(int freq)
    : UServer(freq, 64000000, "console")
  {}
  virtual ~ConsoleServer()
  {}

  virtual void shutdown()
  {
    exit (0);
  }

  virtual void reset()
  {}

  virtual void reboot()
  {}

  virtual ufloat getTime()
  {
    return (ufloat)(urbi::utime()/1000LL);
  }

  virtual ufloat getPower()
  {
    return ufloat(1);
  }

  //! Called to display the header at each coonection start
  virtual void getCustomHeader(int line, char* header, int maxlength)
  {
    const char* banner[] =
      {
	"***      URBI Console " PACKAGE_VERSION_REV "\n",
	"***      (C) 2006 Gostai SAS\n"
      };

    if ((size_t) line < sizeof banner / sizeof banner[0])
      strncpy(header, banner[line], maxlength);
    else
      header[0] = 0;
  }

  virtual
  UErrorValue
  loadFile (const char* filename, UCommandQueue* loadQueue)
  {
    std::ifstream is (filename, std::ios::binary);
    if (!is)
      return UFAIL;

    char buf[URBI_BUFSIZ];
    while (is.good ())
    {
      is.read (buf, URBI_BUFSIZ);
      if (loadQueue->push((const ubyte*)buf, is.gcount()) == UFAIL)
	return UFAIL;
    }
    is.close();

    return USUCCESS;
  }

  virtual
  UErrorValue
  saveFile (const char* filename, const char* content)
  {
    //! \todo check this code
    std::ofstream os (filename);
    os << content;
    os.close ();
    return os.good () ? USUCCESS : UFAIL;
  }

  virtual
  void effectiveDisplay(const char* t)
  {
    printf ("%s",t);
  }
};

int
main (int argc, const char* argv[])
{
  // Input file.
  const char *in = argc == 2 ? argv[1] : "/dev/stdin";

  ConsoleServer s (10);
  s.initialization ();
  UGhostConnection& c = *s.getGhostConnection ();

  if (s.loadFile(in, c.recvQueue ()) != USUCCESS)
  {
    std::cerr << argv[0] << ": failed to process " << in << std::endl;
    return 1;
  }

  c.newDataAdded = true;

  long long startTime = urbi::utime();

  while (true)
    {
      ufloat freq = s.getFrequency() *1000;
      while (urbi::utime() < (startTime + freq))
	usleep (1);
      s.work ();
    }
}
