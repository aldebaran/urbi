#include "libport/compiler.hh"
#include "libport/cstdio"

#include "libport/unistd.h"
#include <list>
#include <algorithm>

#include "network/bsdnet/network.hh"
#include "network/bsdnet/connection.hh"
#include "userver.hh"

namespace Network
{

  /*----------------.
  | TCPServerPipe.  |
  `----------------*/

  class TCPServerPipe: public Pipe
  {
  public:
    TCPServerPipe();
    bool init(int port);
    ~TCPServerPipe();

    virtual std::ostream& print (std::ostream& o) const;

    virtual int readFD();
    virtual int writeFD();

    virtual void notifyWrite();
    virtual void notifyRead();

  private:
    int fd;
    int port;
  };

  TCPServerPipe::TCPServerPipe()
    : Pipe(), fd(-1), port(-1)
  {
  }

  std::ostream&
  TCPServerPipe::print(std::ostream& o) const
  {
    return o
      << "TCPServerPipe "
      << "{ controlFd = " << controlFd
      << ", fd = " << fd
      << ", port = " << port
      << '}';
  }

  TCPServerPipe::~TCPServerPipe ()
  {
    if (fd != -1)
    {
      if (shutdown (fd, SHUT_RDWR))
	perror ("cannot shutdown socket");
      else if (close (fd))
	perror ("cannot close socket");
    }
  }

  bool
  TCPServerPipe::init(int p)
  {
    port = p;
#ifdef WIN32
    // Initialize the socket API.
    WSADATA info;
    // Winsock 1.1.
    if (WSAStartup(MAKEWORD(1, 1), &info))
      return false;
#endif
    // Create the socket.
    fd = socket(AF_INET, SOCK_STREAM, 0);
    ECHO("Created socket: " << fd);
    if (fd == -1)
    {
      perror ("cannot create socket");
      return false;
    }

    // Set the REUSEADDR option to 1 to allow imediate reuse of the port.
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes))
    {
      perror ("setsockopt failed");
      return false;
    }

    // Do not send a SIGPIPE, rather return EPIPE.
    // FIXME: There might be portability issues here.  Note that
    // this is the Mac OSX way to say "MSG_NOSIGNAL".
    if (setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof yes))
    {
      perror ("setsockopt failed");
      return false;
    }


    // Fill in socket address.
    sockaddr_in address;
    memset(&address, 0, sizeof (sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short) port);
    address.sin_addr.s_addr = INADDR_ANY;

    // Bind to port.
    if (bind(fd, (sockaddr*)&address, sizeof (sockaddr)))
    {
      perror ("cannot bind");
      return false;
    }

    // Listen for connections.
    if (listen(fd, 1))
    {
      perror ("cannot listen");
      return false;
    }

    registerNetworkPipe(this);
    return true;
  }


  int
  TCPServerPipe::readFD()
  {
    return fd;
  }

  int
  TCPServerPipe::writeFD()
  {
    return -1;
  }

  void
  TCPServerPipe::notifyRead()
  {
    sockaddr_in client;
    socklen_t asize = sizeof (sockaddr_in);
    int cfd = accept(fd, (sockaddr*) &client, &asize);
    if (cfd == -1)
    {
      perror ("cannot accept");
      return;
    }
    Connection* c = new Connection(cfd);
    ::urbiserver->addConnection(c);
    registerNetworkPipe(c);
  }

  void
  TCPServerPipe::notifyWrite()
  {
    // FIXME: It this really ok to ignore?
  }


  /*-------------------------.
  | Freestanding functions.  |
  `-------------------------*/

  std::ostream&
  operator<< (std::ostream& o, const Pipe& p)
  {
    return p.print (o);
  }

  bool
  createTCPServer(int port)
  {
    TCPServerPipe* tsp = new TCPServerPipe();
    if (!tsp->init(port))
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
    typedef std::list<Pipe*> pipes_type;
    pipes_type pList;
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
    for (pipes_type::iterator i = pList.begin();
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
    // We cannot use a simple loop with a single iterator here,
    // because calls to notifyRead or notifyWrite can call
    // unregisterNetworkPipe which changes pList, resulting in an
    // invalidated iterator.  So we do need two iterators to walk the
    // list.

    for (pipes_type::iterator i = pList.begin(); i != pList.end(); )
    {
      // Next iterator.
      pipes_type::iterator in = i;
      ++in;
      {
	try
	{
	  Pipe& p = **i;
	  int f = p.readFD();
	  if (f >= 0 && FD_ISSET(f, &rd))
	    p.notifyRead();

	  f = p.writeFD();
	  if (f >= 0 && FD_ISSET(f, &wr))
	    p.notifyWrite();
	}
	catch(...)
	{
	  //this can happen if the object was destroyed by the notifyRead
	}
      }
      i = in;
    }
  }

  bool
  selectAndProcess(int usDelay)
  {
    fd_set rd;
    fd_set wr;
    int mx = buildFD(rd, wr);
    timeval tv;
    tv.tv_sec = usDelay / 1000000;
    tv.tv_usec = usDelay - ((usDelay / 1000000) * 1000000);
    int r = select(mx, &rd, &wr, 0, &tv);
    if (r < 0)
      // FIXME: This is bad, we should really do something.
      perror("cannot select");
    else if (!r)
      return false;
    else // 0 < r
    {
#ifndef WIN32
      if (FD_ISSET(controlPipe[0], &rd))
      {
	char buf[128];
	if (read(controlPipe[0], buf, sizeof buf) == -1)
	  perror ("cannot read controlPipe[0]");
      }
#endif
      notify(rd, wr);
    }

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
    pipes_type::iterator i = std::find(pList.begin(), pList.end(), p);
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
    if (pthread_create(pt, 0, &processNetwork, 0))
      perror ("cannot create thread");
#endif
  }



  /*-------.
  | Pipe.  |
  `-------*/

  Pipe::Pipe ()
    : controlFd (-1)
  {}

  void Pipe::trigger()
  {
#ifndef WIN32
    char c = 0;
    if (write(controlFd, &c, 1) == -1)
      perror ("cannot write to controlFD");
#endif
  }
}
