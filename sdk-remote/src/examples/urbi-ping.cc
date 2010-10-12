/*
 * Copyright (C) 2005-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/sys/types.h>
#include <libport/sys/stat.h>
#include <libport/csignal>
#include <libport/unistd.h>
#include <libport/windows.hh>

#include <urbi/uclient.hh>

urbi::UClient* c;
unsigned sendtime;
unsigned mintime;
unsigned maxtime;
unsigned avgtime=0;
unsigned pingCount=0;
bool over=false;
char* rname;
bool received;
unsigned count;
bool flood = false;

static urbi::UCallbackAction
pong(const urbi::UMessage& msg)
{
  unsigned int ptime = msg.client.getCurrentTime() - sendtime;
  if (!pingCount || mintime>ptime)
    mintime = ptime;
  if (!pingCount || maxtime<ptime)
    maxtime = ptime;

  avgtime+=ptime;
  printf("ping reply from %s: seq=%d time=%d ms\n",
         rname, pingCount+1, ptime);
  ++pingCount;
  received=true;
  if (pingCount == count)
    over = true;
  if (flood)
  {
    sendtime = c->getCurrentTime();
    c->send("uping << ping,");
  }
  return urbi::URBI_CONTINUE;
}

static void showstats(int)
{
  if (!pingCount)
    urbi::exit(0);
  printf("rtt min/avg/max %d/%d/%d ms\n",
	 (mintime),
	 (int)(avgtime/(float)pingCount),
	 (maxtime));
  exit(0);
}


int
main(int argc, char* argv[])
{
  signal(SIGINT, showstats);
  if (argc < 2)
  {
    printf("usage: %s robot [msinterval] [count]\n",argv[0]);
    exit(1);
  }

  rname = argv[1];

  // Client initialization
  c = new urbi::UClient(argv[1]);
  c->start();
  if (c->error())
    exit(1);

  int interval = 1000;

  if (2 < argc)
    interval=strtol(argv[2],NULL,0);

  // count initialization
  if (3 < argc)
    count = strtol(argv[3],NULL,0);
  else
    count = 0;

  c->setCallback(&pong,"uping");

  received=true;

  if (interval)
    for (unsigned i = 0; i < count || !count; ++i)
    {
      while (!received)
	usleep(200);
      received=false;
      sendtime = c->getCurrentTime();
      c->send("uping << ping;");
      usleep(interval*1000);
    }
  else
  {
    flood = true;
    c->send("uping << ping;");
  }

  while (!over)
    usleep(1000000);
  showstats(0);
}
