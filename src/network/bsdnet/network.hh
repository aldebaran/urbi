#ifndef NETWORK_HH
# define NETWORK_HH

# include "config.h"

# include "libport/network.h"

namespace Network
{
  class Pipe
  {
  public:
    Pipe();
    virtual ~Pipe() {}
    //returns read fd or -1 if none
    virtual int readFD() = 0;
    virtual int writeFD() = 0;

    virtual void notifyRead() = 0;
    virtual void notifyWrite() = 0;
    void trigger(); ///< trigger demuxer fd set reload

    virtual std::ostream& print (std::ostream& o) const = 0;

  public:
    int controlFd;
  };

  std::ostream& operator<< (std::ostream& o, const Pipe& p);

  /// Build the two fd_sets according to registered connections.
  int buildFD(fd_set& rd, fd_set& wr);

  /// Notify the Pipe object associed with fd sets in the list.
  void notify(fd_set& rd, fd_set& wr);

  void registerNetworkPipe(Pipe* p);
  void unregisterNetworkPipe(Pipe* p);

  bool createTCPServer(int port);

  /// Perform the select with a delay of usedDelay microseconds.
  /// \returns  whether at least one action was performed
  bool selectAndProcess(int usDelay);

  /// Create a thread that will loop forever on selectAndProcess.
  void startNetworkProcessingThread();
}

#endif
