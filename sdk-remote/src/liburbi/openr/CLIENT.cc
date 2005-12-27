/*! \file CLIENT.cc
 */

#include "CLIENT.h"
FILE * _debugfile_ ; 

OID		CLIENT_ID;
antStackRef  ipstackRef;
unsigned int	CLIENTtime;
const char * moduleName = "CLIENT";
extern int		main(int argc, char **argv);

CLIENT::CLIENT()
{
  WhoAmI(&CLIENT_ID);
  CLIENTtime = 0;
}

OStatus
CLIENT::DoInit(const OSystemEvent& event)
{
  NEW_ALL_SUBJECT_AND_OBSERVER;
  REGISTER_ALL_ENTRY;
  SET_ALL_READY_AND_NOTIFY_ENTRY;    

  return oSUCCESS;
}

OStatus
CLIENT::DoStart(const OSystemEvent& event)
{
  ipstackRef = antStackRef("IPStack");

  ENABLE_ALL_SUBJECT;
  ASSERT_READY_TO_ALL_OBSERVER;
  OSYSDEBUG(("liburbi-openr CLIENT started \n"));
  return oSUCCESS;
}

OStatus
CLIENT::DoStop(const OSystemEvent& event)
{
  DISABLE_ALL_SUBJECT;
  DEASSERT_READY_TO_ALL_OBSERVER;
  return oSUCCESS;
}

OStatus
CLIENT::DoDestroy(const OSystemEvent& event)
{
  DELETE_ALL_SUBJECT_AND_OBSERVER;
  return oSUCCESS;
}

void
CLIENT::TCPSendCont(void *msg)
{
  TCPEndpointSendMsg		*tcpSendMsg = (TCPEndpointSendMsg *)msg;
  AiboConnection	*connection = (AiboConnection *)tcpSendMsg->continuation;
  if (tcpSendMsg->error != TCP_SUCCESS)
	{
	  OSYSDEBUG(("%s : antError %d\n",
				 "Failed with TCPSendCont",
				 tcpSendMsg->error));
	  connection->Close();
	}
  else
	connection->continueSend();
}

static int calcTreshold(const char * string) {
  unsigned int primes[]={7,19,127};
  unsigned int sum=0;
  int len = strlen(string)>3? 3: strlen(string);
  for (int i=0;i<len;i++)
    sum += (unsigned int)string[i]*primes[i];
  return (sum%256);
}

void 
CLIENT::NotifyImage(const ONotifyEvent& event) 
{
  char				**argv;
  static int		i = 0;
  static int tresh=0;
  if (tresh==0)
    tresh = calcTreshold(moduleName);
  if (i == tresh)
	{
	  OSYSDEBUG(("Calling main function of liburbi-openr hosted module %s (frame %d)\n", moduleName, tresh));
	  argv = (char **)malloc(3 * sizeof (char *));
	  argv[0] = strdup("urbi_aibo");
	  //argv[1] = strdup("147.250.35.207");
	  argv[1] = strdup("127.0.0.1");
	  argv[2] = 0;
	  main(2, argv);
	}
  i++;
  observer[event.ObsIndex()]->AssertReady();
}

void 
CLIENT::NotifySensor(const ONotifyEvent& event) 
{
  CLIENTtime++;
  observer[event.ObsIndex()]->AssertReady();
}

void
CLIENT::TCPReceiveCont(void *msg)
{
  TCPEndpointReceiveMsg	*tcpRecvMsg = (TCPEndpointReceiveMsg *)msg;
  AiboConnection	*connection = (AiboConnection *)tcpRecvMsg->continuation;
  if (tcpRecvMsg->error != TCP_SUCCESS)
	{
	  if (tcpRecvMsg->error != TCP_CONNECTION_BUSY)
		{
		  OSYSDEBUG(("%s : antError %d\n",
					 "Failed with TCPReceiveCont",
					 tcpRecvMsg->error));
		  connection->Close();
		}
	  return ;
	}

  // notify the UConnection processing function.
  connection->getClient()->received((char *)connection->antRecvData(),
                       tcpRecvMsg->sizeMax);

  // Loop to receive more data.
  connection->Receive();
}

void
CLIENT::TCPCloseCont(void *msg)
{
  TCPEndpointCloseMsg		*tcpCloseMsg = (TCPEndpointCloseMsg *)msg;
  AiboConnection	*connection = (AiboConnection *)tcpCloseMsg->continuation;
}
