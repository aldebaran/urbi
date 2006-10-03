#include <uclient.h>
#include <signal.h>

urbi::UClient *d;

const char * devices[]={
  "legLF1",
  "legLF2",
  "legLF3",
  "legLH1",
  "legLH2",
  "legLH3",
  "legRH1",
  "legRH2",
  "legRH3",
  "legRF1",
  "legRF2",
  "legRF3",
  "neck",
  "headPan",
  "headTilt",
  "tailTilt",
  "tailPan",
  "mouth"
};

int devCount = sizeof (devices) / sizeof (*devices);

urbi::UCallbackAction
command(const urbi::UMessage &msg)
{
  //get command id
  if (msg.type != urbi::MESSAGE_DATA
      || msg.value->type != urbi::DATA_DOUBLE)
    return urbi::URBI_CONTINUE;
  d->send("%s.val = %lf,", msg.tag.c_str (), (double) *msg.value);
  return urbi::URBI_CONTINUE;
}

void endRecord(int sig)
{
  urbi::exit(0);
}

int main(int argc, char * argv[])
{
  if (argc != 3)
    {
      printf("usage: %s sourcerobot destinationrobot [motorstate]\n"
	     "\t Mirror the movements of one robot to the other.\n",
	     argv[0]);
      urbi::exit(1);
    }

  signal(SIGINT,endRecord);

  urbi::UClient c(argv[1]);
  if (c.error())
    urbi::exit(2);

  d = new urbi::UClient(argv[2]);
  if (d->error())
    urbi::exit(2);

  int motorstate=0;

  // FIXME: impossible.
  if (argc>=4)
    motorstate=strtol(argv[3],NULL,0);

  if (!motorstate)
    c.send("motoroff;");

  d->send("motoron;");

  c.send("looptag: loop {");
  for (int i=0;i<devCount-1;i++)
    {
      c.setCallback(command,devices[i]);
      c.send("%s: %s.val&",devices[i], devices[i]);
    }

  c.setCallback(command,devices[devCount-1]);
  c.send("%s: %s.val},",devices[devCount-1], devices[devCount-1]);

  urbi::execute();
}
