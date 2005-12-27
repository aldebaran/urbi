#include <uclient.h>
#include <sys/types.h>
#include <sys/stat.h>

USound snd;
UCallbackAction endProgram(const UMessage &msg) {
  printf("done\n");
  urbi::exit(0);
  return URBI_REMOVE;
}
UCallbackAction soundFormat(const UMessage &msg) {
  UMessage smsg(msg.client, 0,"",msg.systemValue);
  snd = smsg.sound;
  //sem_post(&sem);
  return URBI_REMOVE;
}

int main(int argc, char * argv []) {
  if (argc<3) {
	printf("usage: urbisendsound robot file\n\t file must be in the WAV format\n"); exit(1);
  }

  UClient * uc=new UClient(argv[1]);
  if (uc->error()) exit(2);

  FILE *f;
  if (!strcmp(argv[2],"-")) 
    f = stdin;
  else 
    f = fopen(argv[2],"r");
  if (!f) { printf("error opening file\n"); exit(3); }

  //sem_init(&sem, false, 0);
  //uc->sendCommand(&soundFormat, "speaker.formatlist;");
  soundFormat(UMessage(*uc,0,"a","*** BIN 0 raw 2 16000 16 1"));  //forcing sound format 
  //sem_wait(&sem);

  USound s;

  if (f!=stdin) {
    struct stat st;
    stat(argv[2],&st);
    s.data = (char * )malloc(st.st_size);
    s.soundFormat = SOUND_WAV;	
    s.size = st.st_size;
	fread(s.data, 1,st.st_size, f);
    snd.data = 0;
    convert(s, snd);
    
    uc->setCallback(endProgram,"end");
    printf("sending %d bytes\n",st.st_size);
    uc->sendSound("speaker", snd,"end");
    printf("done, waiting for end of play notification\n");
  }
  else {	
    s.data = (char *)malloc(130000);
    s.soundFormat = SOUND_WAV;
    fread(s.data, 44, 1, f);
    int sz=1;

    while (sz) {
      sz=fread(s.data+44,1,128000,f);
      s.size = sz+44;
      convert(s, snd);
      uc->sendSound("speaker", snd,sz<128000?"end":"void");
      printf("sending %d bytes\n",sz);
    }
  }
  urbi::execute();
}
