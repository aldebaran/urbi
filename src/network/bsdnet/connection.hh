#ifndef CONNECTION_HH
# define CONNECTION_HH

/** \file connection.hh
 *  \brief the linux specialization of the UConnection class of the URBI kernel.
 *  @author Anthony Truchet from a previous work by Arnaud Sarthou  */

# include <sys/types.h>

# include "kernel/utypes.hh"
# include "kernel/uconnection.hh"

# include "network/bsdnet/network.hh"

//! LinuxConnection implements an TCP/IP client connection.
class Connection : public UConnection, public Network::Pipe
{
public:
  // Parameters used by the constructor.
  enum
  {
    MINSENDBUFFERSIZE = 4096,
    MAXSENDBUFFERSIZE = 33554432,
    // This is also the size of the buffer
    PACKETSIZE	     = 16384,
    MINRECVBUFFERSIZE = 4096,
    MAXRECVBUFFERSIZE = 33554432,
  };

  //! Creates a new connection from the connection file descriptor
  /**
   * @param connfd the file descriptor of the underlying socket
   */
  Connection(int connfd);
  virtual ~Connection();
  virtual UErrorValue closeConnection ();

  virtual std::ostream& print (std::ostream& o) const;

  /*ENABLE_URBI_MEM*/

  //! Called when the underlying fd is ready to be read
  void doRead();
  //! Called when the underlying fd is ready to be written
  void doWrite();

  virtual void notifyRead()
  {
    doRead();
  }
  virtual void notifyWrite()
  {
    doWrite();
  }
  virtual UErrorValue send (const ubyte *buffer, int length);

public:
  //! Accessor for the underlying file descriptor
  inline operator int() const
  {
    return fd;
  }

  virtual int readFD()
  {
    return fd;
  }

  virtual int writeFD()
  {
    if (sendQueueRemain()>0)
      return fd;
    else
      return -1;
  }

protected:
  //! Overloading this function is requiered by UConnection
  virtual int effectiveSend (const ubyte *buffer, int length);
  //! The file descriptor of the connection
  int fd;
  //! The reception buffer
  unsigned char read_buff[PACKETSIZE];
};

#endif // !CONNECTION_HH
