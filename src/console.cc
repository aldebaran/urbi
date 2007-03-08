#include "config.h"
#include "version.hh"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include "libport/windows.hh"
#include <fstream>

#include "libport/utime.hh"

#include "kernel/userver.hh"

#include "ughostconnection.hh"

class ConsoleServer
  : public UServer
{
public:
  ConsoleServer(int freq)
    : UServer(freq, 64000000, "console")
  {
    // FIXME: Add support for : in the path.
    if (const char* cp = getenv ("URBI_PATH"))
      path.push_back (cp);
  }

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
    return static_cast<ufloat>(libport::utime() / 1000LL);
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
    printf ("%s", t);
  }
};

int
main (int argc, const char* argv[])
{
  // Input file.
  const char *in = argc == 2 ? argv[1] : "/dev/stdin";

  ConsoleServer s (10);

  s.initialize ();
  UGhostConnection& c = *s.getGhostConnection ();

  if (s.loadFile(in, &c.recvQueue ()) != USUCCESS)
  {
    std::cerr << argv[0] << ": failed to process " << in << std::endl;
    return 1;
  }

  c.newDataAdded = true;

  long long startTime = libport::utime();

  while (true)
  {
    ufloat freq = s.getFrequency() * 1000;
    while (libport::utime() < (startTime + freq))
      usleep (1);
    s.work ();
  }
}
