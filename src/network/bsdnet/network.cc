#include "network/bsdnet/network.hh"
#include "network/bsdnet/connection.hh"

#include "libport/cstdio"

#ifndef WIN32
# include <sys/time.h>
# include <unistd.h>
#endif

#include <sys/types.h>

#include <list>
#include <algorithm>

// In the long run, this should be part of libport, but I don't know
// where actually :( Should it be libport/sys/types.hh?  Or unistd.hh?
// What standard header should do that?

#ifdef WIN32
// On windows, file descriptors are defined as u_int (i.e., unsigned int).
# define LIBPORT_FD_SET(N, P) FD_SET(static_cast<u_int>(N), P)
#else
# define LIBPORT_FD_SET(N, P) FD_SET(N, P)
#endif

namespace Network
{

  class TCPServerPipe: public Pipe
  {
  public:
    TCPServerPipe()
      : fd(-1), port(-1)
    {}
    bool init(int port, const std::string & address);

    virtual int readFD()
    {
      return fd;
    }
    virtual int writeFD()
    {
      return -1;
    }
    virtual void notifyWrite()
    {}
    virtual void notifyRead();

  private:
    int fd;
    int port;
  };

  bool
  TCPServerPipe::init(int port, const std::string &addr)
  {
    this->port = port;
    int rc;
    struct ::sockaddr_in address;

#ifdef WIN32
    /* initialize the socket api */
    WSADATA info;
    rc = WSAStartup(MAKEWORD(1, 1), &info); /* Winsock 1.1 */
    if (rc)
      return false;
#endif
    /* create the socket */
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
      return false;

    /* set the REUSEADDR option to 1 to allow imediate reuse of the port */
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
		   (char *) &yes, sizeof (yes)) < 0)
      return false;

    /* fill in socket address */
    memset(&address, 0, sizeof (struct sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short) port);
    if (addr.empty() || addr == "0.0.0.0")
      address.sin_addr.s_addr = INADDR_ANY;
    else
    {
      //attempt name resolution
      hostent* hp = gethostbyname (addr.c_str());
      if (!hp) //assume IP address in case of failure
        // FIXME: Check that inet_addr did not return INADDR_NONE.
	address.sin_addr.s_addr = inet_addr (addr.c_str());
      else
      {
        /* hp->h_addr is now a char* such as the IP is:
         *    a.b.c.d
         * where
         *    a = hp->h_addr[0]
         *    b = hp->h_addr[1]
         *    c = hp->h_addr[2]
         *    d = hp->h_addr[3]
         * hence the following calculation.  Don't cast this to an int*
         * because of the alignment problems (eg: ARM) and also because
         * sizeof (int) is not necessarily 4.
         */
        uint32_t ip = (hp->h_addr[0] << 24)
          + (hp->h_addr[1] << 16)
          + (hp->h_addr[2] << 8)
          + hp->h_addr[3];
	address.sin_addr.s_addr = ip;
      }
    }
    /* bind to port */
    rc = bind(fd, (struct sockaddr*) &address, sizeof (struct sockaddr));
    if (rc == -1)
      return false;
    /* listen for connections */
    if (listen(fd, 1) == -1)
      return false;

    registerNetworkPipe(this);
    return true;
  }


  void
  TCPServerPipe::notifyRead()
  {
    int cfd;
    struct sockaddr_in client;
    struct hostent* client_info;
    socklen_t asize = sizeof (struct sockaddr_in);
    cfd = accept(fd, (struct sockaddr*) &client, &asize);
    if (cfd == -1)
      return;

    client_info = gethostbyname((char *) inet_ntoa(client.sin_addr));
    Connection* c = new Connection(cfd);
    ::urbiserver->addConnection(c);
    registerNetworkPipe(c);
  }

  bool
  createTCPServer(int port, const char* address)
  {
    TCPServerPipe* tsp = new TCPServerPipe();
    std::string addr;
    if (address)
      addr = address;
    if (!tsp->init(port, addr))
      {
	delete tsp;
	return false;
      }
    return true;
  }


  namespace
  {
#ifndef WIN32
    int controlPipe[2] = {-1, -1};
#endif
    std::list<Pipe*> pList;
  }


  int
  buildFD(fd_set& rd, fd_set& wr)
  {
    FD_ZERO(&rd);
    FD_ZERO(&wr);
    int maxfd = 0;
#ifndef WIN32
    LIBPORT_FD_SET(controlPipe[0], &rd);
    maxfd = controlPipe[0];
#endif
    for (std::list<Pipe*>::iterator i = pList.begin();
	 i != pList.end();
	 ++i)
      {
	int f = (*i)->readFD();
	if (f > 0)
	  LIBPORT_FD_SET(f, &rd);
	if (f > maxfd)
	  maxfd = f;
	int g = (*i)->writeFD();
	if (g > 0)
	  LIBPORT_FD_SET(g, &wr);
	if (g > maxfd)
	  maxfd = g;
      }
    return maxfd + 1;
  }

  void notify(fd_set& rd, fd_set& wr)
  {
    std::list<Pipe*>::iterator in;
    for (std::list<Pipe*>::iterator i = pList.begin();
	 i != pList.end();
	 i = in)
      {
	in = i;
	++in;
	try {
	  Pipe* p = *i;
	  int f = p->readFD();
	  if (f >= 0 && FD_ISSET(f, &rd))
	    p->notifyRead();

	  f = p->writeFD();
	  if (f >= 0 && FD_ISSET(f, &wr))
	    p->notifyWrite();
	}
	catch(...)
	{
	  //this can happen if the object was destroyed by the notifyRead
	}
      }
  }


  bool
  selectAndProcess(int usDelay)
  {
    fd_set rd;
    fd_set wr;
    int mx = buildFD(rd, wr );
    struct timeval tv;
    tv.tv_sec = usDelay / 1000000;
    tv.tv_usec = usDelay - ((usDelay / 1000000) * 1000000);

    int r = select(mx, &rd, &wr, 0, &tv);
    if (!r)
      return false;
    if (r > 0)
      {
#ifndef WIN32
	if (FD_ISSET(controlPipe[0], &rd))
	  {
	    char buf[128];
	    read(controlPipe[0], buf, 128);
	  }
#endif
	notify(rd, wr);
      }
    if (r < 0)
      //XXX this is baad, we should realy do something
      perror("SELECT ERROR:");

    return r > 0;
  }


  void registerNetworkPipe(Pipe* p)
  {
    pList.push_back(p);
#ifndef WIN32
    if (controlPipe[0] == -1)
      pipe(controlPipe);

    p->controlFd = controlPipe[1];
#endif
  }

  void unregisterNetworkPipe(Pipe* p)
  {
    std::list<Pipe*>::iterator i = std::find(pList.begin(), pList.end(), p);
    if (i != pList.end())
      pList.erase(i);
  }

#ifdef WIN32
  enum { delay = 10000 };
  DWORD WINAPI
#else
  enum { delay = 1000000 };
  void*
#endif
  processNetwork(void*)
  {
    while (true)
      selectAndProcess(delay);
  }


  void startNetworkProcessingThread()
  {
#ifdef WIN32
    DWORD tid;
    CreateThread(NULL, 0, processNetwork, 0, 0, &tid);
#else
    pthread_t* pt = new pthread_t;
    pthread_create(pt, 0, &processNetwork, 0);
#endif
  }


  void Pipe::trigger()
  {
#ifndef WIN32
    char  c = 0;
    write(this->controlFd, &c, 1);
#endif
  }
};
