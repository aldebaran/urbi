#include <boost/thread.hpp>

#include "libport/compiler.hh"
#include "libport/cstdio"

#include "libport/unistd.h"
#include <list>
#include <algorithm>

#include "kernel/userver.hh"

#include "network/bsdnet/network.hh"
#include "network/bsdnet/connection.hh"

namespace Network
{

  /*----------------.
  | TCPServerPipe.  |
  `----------------*/

  class TCPServerPipe: public Pipe
  {
  public:
    TCPServerPipe();
    bool init(int port, const std::string & address);
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
  TCPServerPipe::init(int p, const std::string& addr)
  {
    port = p;
#if defined (WIN32) && !defined (__MINGW32__)
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

    // Do not send a SIGPIPE, rather return EPIPE.  See the comment
    // for MSG_NOSIGNAL in connection.cc.
#ifdef SO_NOSIGPIPE
    if (setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof yes))
    {
      perror ("setsockopt failed");
      return false;
    }
#endif

    // Fill in socket address.
    sockaddr_in address;
    memset(&address, 0, sizeof (sockaddr_in));
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
	 * sizeof (int) is not necessarily 4 and also because the result
	 * depends on the endianness of the host.
	 */
	memcpy (&address.sin_addr, hp->h_addr, hp->h_length);
      }
    }

    // Bind to port.
    if (bind(fd, (sockaddr*) &address, sizeof (sockaddr)) == -1)
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
#if !defined (WIN32)
    int controlPipe[2] = { -1, -1 };
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
#if !defined (WIN32)
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
	  if (f >= 0 && LIBPORT_FD_ISSET(f, &rd))
	    p.notifyRead();

	  f = p.writeFD();
	  if (f >= 0 && LIBPORT_FD_ISSET(f, &wr))
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
#if !defined (WIN32)
      if (LIBPORT_FD_ISSET(controlPipe[0], &rd))
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
#if !defined (WIN32)
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
#else
  enum { delay = 1000000 };
#endif

  void processNetwork()
  {
    while (true)
      selectAndProcess(delay);
  }


  void startNetworkProcessingThread()
  {
    boost::thread networkThread(&processNetwork);
  }



  /*-------.
  | Pipe.  |
  `-------*/

  Pipe::Pipe ()
    : controlFd (-1)
  {}

  void Pipe::trigger()
  {
#if !defined (WIN32) || defined (__MINGW32__)
    char c = 0;
    if (write(controlFd, &c, 1) == -1)
      perror ("cannot write to controlFD");
#endif
  }
}
