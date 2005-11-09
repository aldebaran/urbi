#include <uconnection.hh>
#include <userver.hh>


namespace Network {
  
  class Pipe {
 public:
  //returns read fd or -1 if none
  virtual int readFD()=0;
  virtual int writeFD()=0;

  virtual void notifyRead();
  virtual void notifyWrite();
 };
  //build the two fd_sets according to registered connections
  int buildFD(fd_set &rd, fd_set &wr);
  //notify the Pipe object associed with fd sets in the list
  void notify(fd_set &rd, fd_set &wr);

  void registerNetworkPipe(Pipe *p);
  void unregisterNetworkPipe(Pipe *p);
  
  bool createTCPServer(int port);
};
