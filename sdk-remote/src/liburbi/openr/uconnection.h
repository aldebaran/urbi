/*! \file uconnection.h
 */

#ifndef UCONNECTION_H
# define UCONNECTION_H

# include "uqueue.h"

# include <ant.h>
# include <IPAddress.h>
# include <OPENR/ODataFormats.h>
# include <OPENR/OSyslog.h>
# include <EndpointTypes.h>
# include <TCPEndpointMsg.h>

class	UClient;

class UConnection 
{
 public:
  UConnection::UConnection(int			minBufferSize,
						   int			maxBufferSize,
						   int			packetSize,
						   unsigned int	port,
						   const char	*host,
						   UClient		*ref);
  virtual ~UConnection();

 protected:
  unsigned int		port_;
  const char		*host_;
  UClient			*ref_;

  bool				blocked_;

  UQueue			*SendQueue_;
  UQueue			*RecvQueue_;

  int				minBufferSize_;
  int				maxBufferSize_;
  int				packetSize_;

  virtual UErrorValue	effectiveSend(const char *buffer, int len) = 0;

 public:
  UErrorValue		send(const char *buffer, int length);
  UErrorValue		continueSend();
  bool				isBlocked() const;
  void				block();
  void				received(byte *buffer, int length);
};

# include "uclient.h"

#endif /* !UCONNECTION_H */
