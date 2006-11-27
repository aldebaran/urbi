#ifndef NETWORK_HH
# define NETWORK_HH

# include "config.h"

# ifdef WIN32
#  ifndef _WIN32_WINNT
#   define _WIN32_WINNT 0x0400
#  endif
#  define GROUP __GROUP
#  include <winsock2.h>
#  undef GROUP
#  define YYTOKENTYPE
typedef int socklen_t;
# else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <netinet/in.h>
#  include <sys/select.h>
# endif

# include "uconnection.hh"
# include "userver.hh"


namespace Network
{
  class Pipe
  {
  public:

    Pipe() {}
    virtual ~Pipe() {}
    //returns read fd or -1 if none
    virtual int readFD()=0;
    virtual int writeFD()=0;

    virtual void notifyRead()=0;
    virtual void notifyWrite()=0;
    void trigger(); ///< trigger demuxer fd set reload

    int controlFd;
  };

  //build the two fd_sets according to registered connections
  int buildFD(fd_set &rd, fd_set &wr);
  //notify the Pipe object associed with fd sets in the list
  void notify(fd_set &rd, fd_set &wr);

  void registerNetworkPipe(Pipe *p);
  void unregisterNetworkPipe(Pipe *p);

  bool createTCPServer(int port);

  /// Perform the select with a delay of usedDelay microseconds.
  /// \returns  whether at least one action was performed
  bool selectAndProcess(int usDelay);

  /// Create a thread that will loop forever on selectAndProcess.
  void startNetworkProcessingThread();
}

#endif
