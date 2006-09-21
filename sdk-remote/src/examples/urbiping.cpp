#include <uclient.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>


UClient *c;
unsigned int sendtime;
unsigned int mintime;
unsigned int maxtime;
unsigned int avgtime=0;
unsigned int pingCount=0;
bool over=false;
char * rname;
bool received;
int count;

#ifdef WIN32
# include<windows.h>
# define usleep(a) Sleep(a/1000)
#endif


UCallbackAction pong(const UMessage & msg)
{
  unsigned int ptime = msg.client.getCurrentTime() - sendtime;
  if ((!pingCount) || mintime>ptime)
    mintime=ptime;
  if ((!pingCount) || maxtime<ptime)
    maxtime=ptime;

  avgtime+=ptime;
  printf("ping reply from %s: seq=%d time=%d ms\n", rname, pingCount+1, ptime);
  pingCount++;
  received=true;
  if (pingCount==count)
    over = true;
  return URBI_CONTINUE;
}

void showstats(int s)
{
  if (!pingCount)
    urbi::exit(0);
  printf("rtt min/avg/max %d/%d/%d ms\n",
	 (mintime),
	 (int)(avgtime/(float)pingCount),
	 (maxtime));
  exit(0);
}


int main(int argc, char * argv[])
{
  signal(SIGINT, showstats);
  if (argc<2) {
    printf("usage: %s robot [msinterval] [count]\n",argv[0]); exit(1);
  }

  rname = argv[1];

  // Client initialization
  c = new UClient(argv[1]);
  c->start();
  if (c->error()) exit(1);

  int interval=1000;

  if (argc>2) interval=strtol(argv[2],NULL,0);

  // count initialization
  if (argc>3)
    count = strtol(argv[3],NULL,0);
  else
    count = 0;

  c->setCallback(&pong,"uping");

  received=true;

  for (int i=0;i<count || (!count);i++) 
    {
      while (!received)
	usleep(200);
      received=false;
      sendtime = c->getCurrentTime();
      c->send("uping:ping;");
      usleep(interval*1000);
    }

  while (!over) usleep(1000000);
  showstats(0);
}
