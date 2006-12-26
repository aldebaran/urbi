#include "urbi/uclient.hh"
#include <sys/types.h>
#include "libport/sys/stat.h"

urbi::USound snd;

urbi::UCallbackAction
endProgram(const urbi::UMessage&)
{
  printf("done\n");
  urbi::exit(0);
  return urbi::URBI_REMOVE;
}

urbi::UCallbackAction
soundFormat(const urbi::UMessage &msg)
{
  urbi::UMessage smsg(msg.client, 0, "",
		      msg.message.c_str(), std::list<urbi::BinaryData>());
  snd = smsg.value->binary->sound;
  //sem_post(&sem);
  return urbi::URBI_REMOVE;
}

int
main(int argc, char * argv [])
{
  if (argc<3)
    {
      printf("usage: urbisendsound robot file\n\t file must be in the WAV format\n");
      exit(1);
    }

  urbi::UClient uc(argv[1]);
  if (uc.error())
    exit(2);

  FILE *f;
  if (STREQ(argv[2],"-"))
    f = stdin;
  else
    f = fopen(argv[2],"r");
  if (!f)
    {
      printf("error opening file\n");
      exit(3);
    }

  //sem_init(&sem, false, 0);
  //uc->sendCommand(&soundFormat, "speaker.formatlist;");
  soundFormat(urbi::UMessage(uc,0,"a",
			     "*** BIN 0 raw 2 16000 16 1",
			     std::list<urbi::BinaryData>()));  //forcing sound format
  //sem_wait(&sem);

  urbi::USound s;

  if (f!=stdin)
    {
      struct stat st;
      stat(argv[2],&st);
      s.data = static_cast<char *> (malloc (st.st_size));
      s.soundFormat = urbi::SOUND_WAV;
      s.size = st.st_size;
      fread(s.data, 1,st.st_size, f);
      snd.data = 0;
      convert(s, snd);

      uc.setCallback(endProgram,"end");
      printf("sending %d bytes\n", static_cast<int>(st.st_size));
      uc.sendSound("speaker", snd,"end");
      printf("done, waiting for end of play notification\n");
    }
  else
    {
      s.data = static_cast<char *> (malloc (130000));
      s.soundFormat = urbi::SOUND_WAV;
      fread(s.data, 44, 1, f);
      int sz=1;

      while (sz)
      {
	sz=fread(s.data+44,1,128000,f);
	s.size = sz+44;
	convert(s, snd);
	uc.sendSound("speaker", snd,sz<128000?"end":"void");
	printf("sending %d bytes\n",sz);
      }
    }
  urbi::execute();
}
