/*! \file uclient.h
 */

#ifndef UCLIENT_H
# define UCLIENT_H

# include "../uabstractclient.h"

class	AiboConnection;

class UClient: public UAbstractClient
{
 public:

  UClient(const char *host, int port = URBI_PORT, int buflen = 128000);

  virtual ~UClient();

  //! For compatibility with older versions of the library
  void start() {}
 
  virtual void	printf(const char *format, ...);
  virtual unsigned int getCurrentTime();
  virtual void lockSend();
  virtual void unlockSend();
  void doProcessRecvBuffer() {processRecvBuffer();}
 protected:


  virtual int	effectiveSend(const void * buffer, int size);
  virtual bool	canSend(int size);

  virtual void lockList();
  virtual void unlockList();


 private:
  AiboConnection		*connection;

 public:
  char	*getRecvBuffer() const;
  int	getRecvBufferPosition() const;
  void	setRecvBuffer(byte *buf, int length);
  void	setRecvBufferPosition(int pos);

  void  received(const char * data, int size);
};



# include "aiboconnection.h"

#endif /* !UCLIENT_H */
