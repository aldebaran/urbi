#ifndef CONNECTION_HH
# define CONNECTION_HH

# include <sys/types.h>

/** \file Connection.h.cpp
 *  \brief the linux specialization of the UConnection class of the URBI kernel.
 *  @author Anthony Truchet from a previous work by Arnaud Sarthou  */

# include "uconnection.hh"
# include "utypes.hh"
# include "network/bsdnet/network.hh"

//! LinuxConnection implements an TCP/IP client connection.
class Connection : public UConnection, public Network::Pipe
{
public:
  // Parameters used by the constructor.
  enum
  {
    MINSENDBUFFERSIZE = 4096,
    MAXSENDBUFFERSIZE = 1048576,
    // This is also the size of the buffer
    PACKETSIZE	     = 16384,
    MINRECVBUFFERSIZE = 4096,
    MAXRECVBUFFERSIZE = 32768,
  };

  //! Creates a new connection from the connection file descriptor
  /**
   * @param fd the file descriptor of the underlying socket
   * @param clientinfo a pointer to the informations about the client connected
   * NOTE : the LinuxConnection stole the property of the struct hostent and
   * is thus responsible for its deletion, and the declaration of its allocation*/
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
  virtual UErrorValue send  (const ubyte *buffer, int length);

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
