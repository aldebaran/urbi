/*! \file CLIENT.h
 */

#ifndef CLIENT_H
# define CLIENT_H

# include "uclient.h"
# include "def.h"

# include <OPENR/OObject.h>
# include <OPENR/OSubject.h>
# include <OPENR/OObserver.h>
# include <OPENR/core_macro.h>
# include <OPENR/OSyslog.h>
# include <OPENR/OPENRAPI.h> 
# include <OPENR/OFbkImage.h>
# include <ant.h>

# include <sys/time.h>
# include <string.h>

#define debug(a...)  
//{OSYSPRINT((a)); int j=0;for (int i=0;i<150000;i++) { j=j+sin(i+j);}}

class	CLIENT : public OObject
{
 public:

  CLIENT();
  virtual ~CLIENT() {};

  virtual OStatus		DoInit(const OSystemEvent& event);
  virtual OStatus		DoStart(const OSystemEvent& event);
  virtual OStatus		DoStop(const OSystemEvent& event);
  virtual OStatus		DoDestroy(const OSystemEvent& event);

  OSubject		*subject[numOfSubject];
  OObserver		*observer[numOfObserver];     

  void	TCPSendCont(void* msg);
  void	TCPReceiveCont(void* msg);
  void	TCPCloseCont  (void* msg);
  void	NotifySensor(const ONotifyEvent& event);
  void	NotifyImage(const ONotifyEvent& event);
};

#endif /* !CLIENT_H */

