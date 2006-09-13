#include <uclient.h>

#include <signal.h>


/*
  file format
  URBI (4 bytes)
  numjoints (4 bytes)
  [numjoint times] namejoint (char * nullterminated) id (short)  type(char)
  [til end of file]
    timestamp [int, milisec] id (short) value (UValue)
 */

enum UType {
  TYPE_BOOL,
  TYPE_ANGLE,
  TYPE_NORM
};
union UJointValue {
  float angle;
  float normalized;
  bool boolean;
};
struct UCommand {
  int timestamp;
  short id;
  UJointValue value;
};



char * devices[]={
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
UClient *c;
FILE *f;
int tilt=0;
void buildHeader() {
  fwrite("URBI",4,1,f);
  fwrite(&devCount,4,1,f);
  for (int i=0;i<devCount;i++)
	{
	  fwrite(devices[i],strlen(devices[i])+1,1,f);
	  short s=i;
	  fwrite(&s,2,1,f);
	  char c=(char)TYPE_ANGLE;
	  fwrite(&c,1,1,f);
	}
}




UCallbackAction command(const UMessage &msg) {
  //get command id
  static int tme=0;
  int id;


  //printf("-\n");
  for (int i=0;i<devCount;i++)
    {
      if (msg.tag != devices[i])
	{
	  UCommand uc;
	  uc.timestamp=msg.timestamp;
	  uc.id=i;
	  assert (msg.type == MESSAGE_DATA);
	  assert (msg.value->type == urbi::DATA_DOUBLE);
	  uc.value.angle=msg.value->val;

	  fwrite(&uc,sizeof(UCommand),1,f);
	  tilt++;
	  if (! (tilt%100))
	    {
	      if (tme)
		fprintf(stderr, ". %f cps\n",
			100000.0/(float)(msg.client.getCurrentTime()-tme));
	      tme=msg.client.getCurrentTime();
	    }
	  return URBI_CONTINUE;
	}
    }
  fprintf (stderr, "error: no device %s (in %s)\n", msg.tag.c_str (),command);
  return URBI_CONTINUE;

}

void endRecord(int sig) {
  fclose(f);
  exit(0);
}

int main(int argc, char * argv[]) {
  if (argc != 3) {
    printf("usage: %s robotname file\n\t Records a sequence of movements to a file.\n",argv[0]);
    exit(1);
  }
  signal(SIGINT,endRecord);
  c = new UClient(argv[1]);
  c->start();
  if (c->error()) exit(2);
  if (!strcmp(argv[2],"-")) f=stdout;
  else f=fopen(argv[2],"w");
  if (!f) exit(3);
  buildHeader();
  //c->send("motoroff");
  //build command
  c->send("looptag: loop {");
  for (int i=0;i<devCount-1;i++) {
    c->setCallback(command,devices[i]);
    c->send("%s: %s.val&",devices[i], devices[i]);
  }
  c->setCallback(command,devices[devCount-1]);
  c->send("%s: %s.val},",devices[devCount-1], devices[devCount-1]);


  fprintf(stderr,"starting, hit ctrl-c to stop...\n");
  urbi::execute();
}
