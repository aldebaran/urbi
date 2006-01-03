/****************************************************************************
 * $Id: urbispeak.cpp,v 1.7 2005/10/03 12:46:38 nottale Exp $
 *
 * Sample demonstration of URBI capabilities.
 *
 * Copyright (C) 2004 Jean-Christophe Baillie.  All rights reserved.
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
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "uclient.h"
#include "usoftdevice.h"
char * festivalBin;
char * festivalScript;

UBinary festival_make_wav(string text) {
  //let's try to be reentrant, although it's not realy necessary

  UBinary result;
  result.size = 0;
  result.message = strdup("error");
  result.data = 0;


  char inFile[1024];
  char outFile[1024];
#ifdef WIN32
  tmpnam_s(inFile, 1024);
  int i = open(inFile, O_WRONLY | O_CREAT);
  tmpnam_s(outFile, 1024);
#else
  strcpy(inFile,"/tmp/urbispeak-XXXXXX");
  int i = mkstemp(inFile);
  strcpy(outFile,"/tmp/urbispeak-out-XXXXXX");
  int j = mkstemp(outFile);
  close(j);
#endif
  write(i,text.c_str(),  text.length());
  close(i);
  //build festival command line
  char cmdline[1024];
  sprintf(cmdline,"%s --script %s -o %s %s", festivalBin, festivalScript, outFile, inFile);

  //execute
#ifdef WIN32
  WinExec(cmdline, SW_SHOW);
#else
  system(cmdline);
#endif
  //recover wav
  FILE *f = fopen(outFile,"r");
  if (!f)
    return result;
  
  struct stat s;
  stat(outFile, &s);
  result.data = malloc(result.size = s.st_size);
  fread(result.data, 1, result.size, f);
  fclose(f);
  result.message = strdup("wav");
  return result;
}



int main(int argc, const char * argv[]) {
  if (argc!=2) {
    fprintf(stderr,"usage: %s robotName\n\t uses festival to gives text-to-speech capability to URBI\n", argv[0]);
    return -1;
  }
  festivalBin = "festival";
  festivalScript = "text2wav";
  
  UClient * cl = new UClient(argv[1]);
  registerDeviceFunction(*cl, "speech","say", devicecallback(&festival_make_wav));
  fprintf(stderr,"Running, bound to function speech.say(text). Use with 'speaker = speech.say(text)\n");
  urbi::execute();
}
