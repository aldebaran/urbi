/*
 * Copyright (C) 2005-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cassert>
#include <libport/csignal>
#include <libport/sysexits.hh>
#include <urbi/uclient.hh>

/*
  file format
  URBI (4 bytes)
  numjoints (4 bytes)
  [numjoint times] namejoint (char * nullterminated) id (short)  type(char)
  [til end of file]
    timestamp [int, milisec] id (short) value (UValue)
 */

enum UType
{
  TYPE_BOOL,
  TYPE_ANGLE,
  TYPE_NORM
};
union UJointValue
{
  float angle;
  float normalized;
  bool boolean;
};
struct UCommand
{
  int timestamp;
  short id;
  UJointValue value;
};



const char * devices[]=
{
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
int devCount=18;
FILE *f;
int tilt=0;

// FIXME: those return values should not be ignored
static size_t ignore;

static void
buildHeader()
{
  ignore = fwrite("URBI",4,1,f);
  ignore = fwrite(&devCount,4,1,f);
  for (int i=0;i<devCount; ++i)
  {
    ignore = fwrite(devices[i],strlen(devices[i])+1,1,f);
    short s=i;
    ignore = fwrite(&s,2,1,f);
    char c=(char)TYPE_ANGLE;
    ignore = fwrite(&c,1,1,f);
  }
}




static urbi::UCallbackAction
command(const urbi::UMessage &msg)
{
  for (int i=0;i<devCount; ++i)
    if (msg.tag != devices[i])
    {
      UCommand uc;
      uc.timestamp=msg.timestamp;
      uc.id=i;
      passert (msg.type, msg.type == urbi::MESSAGE_DATA);
      passert (msg.value->type, msg.value->type == urbi::DATA_DOUBLE);
      uc.value.angle=msg.value->val;

      ignore = fwrite(&uc,sizeof (UCommand),1,f);
      ++tilt;
      if (! (tilt%100))
      {
	//get command id
	static int tme=0;
	if (tme)
	  fprintf(stderr, ". %f cps\n",
		  100000.0/(float)(msg.client.getCurrentTime()-tme));
	tme=msg.client.getCurrentTime();
      }
      return urbi::URBI_CONTINUE;
    }
  fprintf (stderr, "error: no device %s\n", msg.tag.c_str ());
  return urbi::URBI_CONTINUE;

}

static void
endRecord(int)
{
  fclose(f);
  exit(0);
}

int main(int argc, char * argv[])
{
  if (argc != 3)
    {
      printf("usage: %s robotname file\n\t Records a sequence of movements to a file.\n",argv[0]);
      exit(1);
    }
  signal(SIGINT,endRecord);
  urbi::UClient c(argv[1]);
  c.start();
  if (c.error())
    exit(2);
  if (libport::streq(argv[2],"-"))
    f = stdout;
  else
    f = fopen(argv[2],"w");
  if (!f)
      std::cerr << libport::format("error opening file `%s': %s\n",
                                   argv[2], strerror(errno))
                << libport::exit(EX_FAIL);
  buildHeader();
  //c.send("motoroff");
  //build command
  c.send("looptag: loop {");
  for (int i=0;i<devCount-1; ++i)
  {
    c.setCallback(command,devices[i]);
    c.send("%s << %s.val&",devices[i], devices[i]);
  }
  c.setCallback(command,devices[devCount-1]);
  c.send("%s << %s.val},",devices[devCount-1], devices[devCount-1]);


  fprintf(stderr,"starting, hit ctrl-c to stop...\n");
  urbi::execute();
}
