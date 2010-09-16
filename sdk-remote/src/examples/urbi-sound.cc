/*
 * Copyright (C) 2005-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/* This simple demonstration program save to a file or send to the
 * speakers sounds coming from an Urbi server.  The data written is
 * the raw data as sent by the server (ers7 = 16000Hz stereo 16bit).
 * You can use the sox program to convert it.
 */

#include <libport/cstdio>
#include <libport/ctime>
#include <libport/sys/types.h>
#include <libport/sys/stat.h>

#include <libport/fcntl.h>
#include <urbi/uclient.hh>
#include <urbi/uconversion.hh>

#ifndef WIN32
# include <sys/ioctl.h>
# include <sys/soundcard.h>
static const char *device="/dev/dsp";
#endif

// FIXME: those return values should not be ignored
static size_t ignore;

struct wavheader
{
  char riff[4];
  int length;
  char wave[4];
  char fmt[4];
  int lnginfo;
  short one;
  short channels;
  int freqechant;
  int bytespersec;
  short bytesperechant;
  short bitperchannel;
  char data[4];
  int datalength;
};



char * fname;
int imcount;
FILE *  file;
bool outtodsp;
bool withheader;
bool waswithheader;
int totallength;

static urbi::UCallbackAction
endProgram(const urbi::UMessage&)
{
  if (waswithheader)
  {
    //fclose(file);
    //file = fopen(fname, "w");
    // seek to set correct size in wav header
    if (fseek(file,offsetof(wavheader, length),SEEK_SET)==-1)
      {
	fprintf(stderr, "warning, can't seek output file (stdout?), wav header will be incorrect!\n");
	exit(0);
    }
    int len = totallength  - 8;
    ignore = fwrite(&len, 1, 4, file);
    fseek(file,offsetof(wavheader, datalength),SEEK_SET);
    len = totallength - 44;
    ignore = fwrite(&len, 1, 4, file);
    fclose(file);
    exit(0);
  }
  exit(0);
  return urbi::URBI_CONTINUE;
}

static urbi::UCallbackAction
getSound(const urbi::UMessage& msg)
{
  static urbi::USound out;
  static bool initialized=false;
  if (!initialized)
  {
    totallength = 0;
    initialized = true;
    out.data = 0;
    out.size = 0;
    if (outtodsp)
    {
      out.channels = 2;
      out.sampleSize = 16;
      out.rate = 16000;
      out.sampleFormat = urbi::SAMPLE_SIGNED;
    }
    else
    {
      out.channels = 0;
      out.sampleSize = 0;
      out.rate = 0;
      out.sampleFormat = (urbi::USoundSampleFormat)0;
    }
  }

  if (msg.type != urbi::MESSAGE_DATA
      || msg.value->type != urbi::DATA_BINARY
      || msg.value->binary->type != urbi::BINARY_SOUND)
    return urbi::URBI_CONTINUE;
  out.soundFormat =
    withheader? urbi::SOUND_WAV : urbi::SOUND_RAW;
  withheader = false;
  convert(msg.value->binary->sound, out);
  totallength += out.size;
  ignore = fwrite(out.data, out.size, 1, file);
  return urbi::URBI_CONTINUE;
}


int main(int argc, char *argv[])
{
  const char *usage = "usage:  urbisound robot milisecondtime  :plays sound recorded by the aibo to /dev/dsp\n"
    "\turbisound robot milisecondtime file [withoutheader] : write recorded sound to a file, with a wav header except if argument withoutheader is set to anything\n";
  //16000 1 16
  if (argc != 3 && argc !=4 && argc != 5)
  {
    printf("%s", usage);
    exit(1);
  }

  int time = strtol(argv[2], NULL, 0);

  if (argc >= 4)
  {
    outtodsp = false;
    if (libport::streq(argv[3],"-"))
      file=stdout;
    else
      file = fopen(argv[3], "wb+");
    fname = argv[3];
    if (file==0)
    {
      printf("error creating file\n");
      exit(2);
    }

    withheader = argc !=5;
  }
  else
  {
#ifdef WIN32
    printf("output to soundcard not supported under windows\n");
    exit(2);
#else
    outtodsp = true;
    withheader = false;
    file = fopen(device, "wb" );
    if (file==0)
    {
      printf("error opening device\n"), exit(2);
    }
    int f = fileno(file);
    int param;

    param=16;
    if (ioctl(f, SNDCTL_DSP_SAMPLESIZE, &param)==-1)
      {
	fprintf(stderr,"failed to set sample size for %s\n", device);
	exit(1);
      }

    param=1;
    if (ioctl (f, SNDCTL_DSP_STEREO, &param)==-1)
      {
	fprintf(stderr,"failed to set stereo for %s\n", device);
	exit(1);
      }

    param=16000;
    if (ioctl (f, SNDCTL_DSP_SPEED, &param) == -1)
      {
	fprintf(stderr,"failed to set speed for %s\n", device);
	exit(1);
      }
#endif
  }

  waswithheader = withheader;
  urbi::UClient client (argv[1]);
  if (client.error())
    exit(0);

  client.setCallback(getSound, "usound");
  client.setCallback(endProgram, "end");

  client.send("var end = Channel.new(\"end\"); var h = WeakPointer.new;var usound = Channel.new(\"usound\"); micro.getSlot(\"val\").notifyChange(h, closure() { usound << micro.val });"
	      "{ sleep(%d); end << 1 }, ", time);
  urbi::execute();
}
