#include <sys/time.h>
#include <time.h>	     
#include "version.hh"
#include "userver.h"
#include "ughostconnection.h"

#ifdef WIN32
inline long long utime()
{
  static long long pfreq= 0LL; //cps
  static long long base = 0LL;
  if (pfreq == 0)
    {
      QueryPerformanceFrequency((LARGE_INTEGER *)&pfreq);
      QueryPerformanceCounter((LARGE_INTEGER *)&base);
      std::cerr <<"perfcounter at frequency of "<<pfreq<<"\n";
    }
  long long val;
  QueryPerformanceCounter((LARGE_INTEGER *)&val);
  return ((val-base) * 1000000LL)/ pfreq;
}
#else
#include <sys/time.h>
#include <time.h>
inline long long utime()
{
  static long long start = 0;
  static const long long BIGDELTA = 30LL*365LL*24LL*3600LL;
  struct timeval tv;
  gettimeofday(&tv, 0);
  if (start == 0)
    start = (tv.tv_sec-BIGDELTA)*1000000+tv.tv_usec;
  return (tv.tv_sec-BIGDELTA)*1000000+tv.tv_usec - start;
}
#endif

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
    return (ufloat)(utime()/1000LL);
  }

  virtual ufloat getPower() 
  {
    return ufloat(1);
  }

  //! Called to display the header at each coonection start
  virtual void getCustomHeader(int line, char* header, int maxlength)
  {
    switch (line)
      {
      case 0:
	strncpy(header,
		"***      URBI Console Rev. " PACKAGE_REVISION "\n",
		maxlength);
	break;
      default:
	header[0] = 0;
      }
  }

  virtual
  UErrorValue
  loadFile (const char *filename, UCommandQueue* loadQueue)
  {
    //! \todo check this code
    FILE *f;
    int nbread;
    char buf[1024];
    f = fopen(filename,"r");
    if (!f)
    return UFAIL;

    while (!feof(f))
      {
	nbread = fread (buf,
			sizeof buf[0], sizeof buf / sizeof buf[0],
			f);
	if (loadQueue->push((const ubyte*)buf,nbread) == UFAIL)
	  return (UFAIL);
      }
    
    fclose(f);
    return USUCCESS;
  }

  virtual 
  UErrorValue 
  saveFile (const char *filename, const char * content)
  {
    //! \todo check this code
    FILE *f;
    f = fopen(filename, "w");
    if (!f)
      return UFAIL;

    int sent = 0;
    int toSend = strlen(content);
    int nbSent;
    
    while (sent < toSend)
      {
	nbSent = fwrite((const void*)(content+sent),
			sizeof(char),toSend-sent,f);
	sent = sent + nbSent;
      }
    
    fclose(f);
    return USUCCESS;
  }

  virtual
  void effectiveDisplay(const char* t) 
  {
    printf ("%s",t);
  }
};

int
main ()
{
  ConsoleServer s (10);
  s.initialization ();
  UGhostConnection& c = *s.getGhostConnection ();
  if (s.loadFile("/dev/stdin", c.recvQueue ()) != USUCCESS)
    return 1;
  c.newDataAdded = true;

  long long startTime = utime();

  while (true)
    {
      ufloat freq = s.getFrequency() *1000;
      while (utime() < (startTime + freq))
	usleep (1);
      s.work ();
    }
}
