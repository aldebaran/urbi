#include <list>
#include "network.h"
namespace Network {
  class TCPServerPipe: public Pipe {
  public:
    TCPServerPipe(): fd(-1), port(-1) {}
    bool init(int port);
    
    virtual int readFD() {return fd;}
    virtual int writeFD() {return -1;}
    virtual void doWrite() {}
    virtual void doRead();

  private:
    int fd;
    int port;
  };

  bool TCPServerPipe::init(int port) {
    this->port = port;
    int rc;
    struct sockaddr_in address;
    
#ifdef WIN32
    /* initialize the socket api */
    WSADATA info;
    rc = WSAStartup(MAKEWORD(1,1),&info); /* Winsock 1.1 */
    if (rc!=0) {
      return false;
    }
#endif
    /* create the socket */
    fd = socket(AF_INET,SOCK_STREAM,0);
    if (fd==-1) {
      return false;
    }
    
    /* set the REUSEADDR option to 1 to allow imediate reuse of the port */
    int yes = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char *) &yes, sizeof (yes)) < 0) {
      return false;
    }
    
    /* fill in socket address */
    memset(&address,0,sizeof(struct sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)port);
    address.sin_addr.s_addr = INADDR_ANY;
    /* bind to port */
    rc = bind(fd,(struct sockaddr *)&address,sizeof(struct sockaddr));
    if (rc==-1) {
      return false;
    }
    /* listen for connections */
    if (listen(fd,1)==-1) {
      return false;
    }
    
    registerNetworkPipe(this);
    return true;
  }


  void TCPServerPipe::doRead() {
    int cfd;
    struct sockaddr_in client;
    struct hostent* client_info;
    socklen_t asize = sizeof(struct sockaddr_in);
    cfd = accept(sfd, (struct sockaddr *)&client, &asize);
    if (cfd==-1) {
      return ;
    }
    client_info = gethostbyname((char *)inet_ntoa(client.sin_addr));
    Connection *c = new Connection(cfd);
    registerNetworkPipe(c);
  }

  using std::list;
  list<Pipe *> pList;
  

  int buildFD(fd_set &rd, fd_set &wr) {
    FD_ZERO(&rd);
    FD_ZERO(&wr);
    int maxfd=0;
    for (list<Pipe *>::iterator i = pList.begin(); i != pList.end(); i++) {
      int f= (*i)->readFD();
      if (f>0)
	FD_SET(f,&rd);
      if (f>maxfd)
	maxfd = f;
      f= (*i)->writeFD();
      if (f>0)
	FD_SET(f,&wr);
      if (f>maxfd)
	maxfd = f;
    }
    return maxfd+1;
  }

  void notify(fd_set &rd, fd_set &wr) {
    for (list<Pipe *>::iterator i = pList.begin(); i != pList.end(); i++) {
      int f= (*i)->readFD();
      if (f>=0 && FD_ISSET(f,&rd))
	(*i)->notifyRead();
      f= (*i)->writeFD();
      if (f>=0 && FD_ISSET(f,&wr))
	(*i)->notifyWrite();
    }
  }
  void registerNetworkPipe(Pipe *p) {
    pList.push_back(p);
  }
  void unregisterNetworkPipe(Pipe *p) {
    list<Pipe *>::iterator i = find(pList.begin(), pList.end(), p);
    if (i != pList.end())
      pList.erase(i); 
  }
};
