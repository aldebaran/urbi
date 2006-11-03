/****************************************************************************
 * $Id: urbisound.cpp,v 1.9 2005/09/21 06:45:36 nottale Exp $
 *
 * Sample demonstration of URBI sound capabilities.
 *
 * Copyright (C) 2004, 2006 Jean-Christophe Baillie.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**********************************************************************/

/* This simple demonstration program save to a file or send to the
 * speakers sounds coming from an Urbi server.  The data written is
 * the raw data as sent by the server (ers7 = 16000Hz stereo 16bit).
 * You can use the sox program to convert it.
 */

#include <cstdio>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include "uclient.h"

#ifndef WIN32
# include <sys/ioctl.h>
# include <sys/soundcard.h>
static const char *device="/dev/dsp";
#endif

struct wavheader {
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

urbi::UCallbackAction
endProgram(const urbi::UMessage &msg)
{
  if (waswithheader) {
    //fclose(file);
    //file = fopen(fname, "w");
    // seek to set correct size in wav header
    if (fseek(file,offsetof(wavheader, length),SEEK_SET)==-1)
      {
	fprintf(stderr, "warning, can't seek output file (stdout?), wav header will be incorrect!\n");
	exit(0);
    }
    int len = totallength  - 8;
    fwrite(&len, 1, 4, file);
    fseek(file,offsetof(wavheader, datalength),SEEK_SET);
    len = totallength - 44;
    fwrite(&len, 1, 4, file);
    fclose(file);
    exit(0);
  }
  exit(0);
  return urbi::URBI_CONTINUE;
}

urbi::UCallbackAction
getSound(const urbi::UMessage &msg)
{
  static urbi::USound out;
  static bool initialized=false;
  if (!initialized) {
    totallength = 0;
    initialized = true;
    out.data = 0;
    out.size = 0;
    if (outtodsp) {
      out.channels = 2;
      out.sampleSize = 16;
      out.rate = 16000;
      out.sampleFormat = urbi::SAMPLE_SIGNED;
    }
    else {
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
  out.soundFormat = withheader? urbi::SOUND_WAV : urbi::SOUND_RAW;
  withheader = false;
  convert(msg.value->binary->sound, out);
  totallength += out.size;
  fwrite(out.data, out.size, 1, file);
  return urbi::URBI_CONTINUE;
}


int main(int argc, char *argv[])
{
  const char *usage = "usage:  urbisound robot milisecondtime  :plays sound recorded by the aibo to /dev/dsp\n"
    "\turbisound robot milisecondtime file [withoutheader] : write recorded sound to a file, with a wav header except if argument withoutheader is set to anything\n";
  //16000 1 16
  if (argc != 3) {
    printf(usage);
    exit(1);
  }

  int time = strtol(argv[2], NULL, 0);

  if (argc >= 4) {
    outtodsp = false;
    if (!strcmp(argv[3],"-"))
      file=stdout;
    else
      file = fopen(argv[3], "wb+");
    fname = argv[3];
    if (file==0) {
      printf("error creating file\n");
      exit(2);
    }

    if (argc==5)
      withheader = false;
    else
      withheader = true;
  }
  else {
#ifdef WIN32
    printf("output to soundcard not supported under windows\n");
    exit(2);
#else
    outtodsp = true;
    withheader = false;
    file = fopen(device, "wb" );
    if (file==0) {
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

  client.send("loopsound:loop usound: micro.val ,"
	      "{ wait(%d); stop loopsound; wait(1000); end:ping }, ", time);
  urbi::execute();
}
