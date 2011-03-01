/*
 * Copyright (C) 2005-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/sys/types.h>
#include <libport/sysexits.hh>
#include <libport/sys/stat.h>

#include <urbi/uclient.hh>
#include <urbi/uconversion.hh>

// FIXME: those return values should not be ignored
static size_t ignore;

urbi::USound snd;

static urbi::UCallbackAction
endProgram(const urbi::UMessage&)
{
  std::cout << "done" << std::endl;
  urbi::exit(0);
  return urbi::URBI_REMOVE;
}

int
main(int argc, char * argv [])
{
  if (argc<3)
  {
    printf("usage: urbi-sendsound robot file\n"
           "\t file must be in the WAV format\n");
    exit(1);
  }

  urbi::UClient uc(argv[1]);
  if (uc.error())
    exit(2);

  FILE *f;
  if (libport::streq(argv[2],"-"))
    f = stdin;
  else
    f = fopen(argv[2],"r");
  if (!f)
    std::cerr << libport::format("error opening file `%s': %s\n",
                                 argv[2], strerror(errno))
              << libport::exit(EX_FAIL);

  //sem_init(&sem, false, 0);
  //uc->sendCommand(&soundFormat, "speaker.formatlist;");
  snd.data = 0;
  snd.size = 0;
  snd.soundFormat = urbi::SOUND_RAW;
  snd.channels = 2;
  snd.rate = 16000;
  snd.sampleSize = 16;
  snd.sampleFormat = urbi::SAMPLE_SIGNED;
  //sem_wait(&sem);

  urbi::USound s;
  s.soundFormat = urbi::SOUND_WAV;

  if (f == stdin)
  {
    s.data = static_cast<char *> (malloc (130000));
    ignore = fread(s.data, 44, 1, f);
    while (int sz = fread(s.data+44,1,128000,f))
    {
      s.size = sz+44;
      convert(s, snd);
      uc.sendSound("speaker", snd, sz < 128000 ? "end" : "void");
      printf("sending %d bytes\n", sz);
    }
  }
  else
  {
    struct stat st;
    stat(argv[2], &st);
    s.data = static_cast<char *> (malloc (st.st_size));
    s.size = st.st_size;
    ignore = fread(s.data, 1,st.st_size, f);
    snd.data = 0;
    convert(s, snd);
    uc.setCallback(endProgram, "end");
    printf("sending %d bytes\n", static_cast<int>(st.st_size));
    uc.sendSound("speaker", snd, "end");
    printf("done, waiting for end of play notification\n");
  }
  urbi::execute();
}
