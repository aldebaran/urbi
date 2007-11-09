/****************************************************************************
 * $Id: urbiimage.cpp,v 1.13 2005/09/30 17:48:00 nottale Exp $
 *
 * Sample image acqusition urbi client.
 *
 * Copyright (C) 2004, 2006, 2007 Jean-Christophe Baillie.  All rights reserved.
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

/*
 * This simple demonstration program display or save images from an Urbi server.
 */

#include "libport/cstdio"
#include <csignal>

#include "urbi/usyncclient.hh"
#include "monitor.h"

int imcount;
int format;
Monitor *mon=0;
unsigned char * buffer=NULL;
float scale;
urbi::UImage im;

/* Our callback function */
urbi::UCallbackAction
showImage(const urbi::UMessage &msg)
{
  if (msg.type != urbi::MESSAGE_DATA
      || msg.value->type != urbi::DATA_BINARY
      || msg.value->binary->type != urbi::BINARY_IMAGE)
    return urbi::URBI_CONTINUE;

  urbi::UImage& img = msg.value->binary->image;
  im.width = static_cast<int>(static_cast<float>(img.width) * scale);
  im.height = static_cast<int>(static_cast<float>(img.height) * scale);
  convert(img, im);
  
  int sz = 3*im.width*im.height;
  static int tme = 0;
  /* Calculate framerate. */
  if (!(imcount % 20))
    {
      if (tme)
	{
	  float it = msg.client.getCurrentTime() - tme;
	  it = 20000.0 / it;
	  printf("***Framerate: %f fps***\n", it);
	}
      tme = msg.client.getCurrentTime();
    }

  if (!mon)
    mon = new Monitor(im.width, im.height, "image");

  mon->setImage((bits8 *) im.data, sz);
  ++imcount;
  return urbi::URBI_CONTINUE;
}

void
closeandquit (int)
{
  delete urbi::getDefaultClient();
  urbi::exit(0);
}


void usage(const char * n) 
{
   fprintf(stderr,
	      "usage: %s [options] format robotname\n"
	      "  Displays images from an urbi server, or save one image if\n" 
	      "    -o is given\n"
	      "  Options:\n"
	      "    -p port   : use port instead of default urbi server port\n"
	      "    -f period : query images at given period (in milliseconds)\n"
	      "    -r        : use reconstruct mode (for aibo)\n"
	      "    -j factor : jpeg compression factor (from 0 to 100, def 70\n"
	      "    -d device : query image on device.val (default: camera.val\n"
	      "    -o file   : query and save one image to file\n"
	      "    -s scale  : rescale image with given factor (display only)\n"
	      "  transfer Format : jpeg=transfer jpeg, raw=transfer raw\n"
	      "  save     Format : rgb , ycrcb, jpeg, ppm\n", n
	      );
}

int
main (int argc, char *argv[])
{
  signal(SIGINT, closeandquit);
  int frequency = 0;
  const char * device = "camera";
  char * fileName = 0;
  bool reconstruct = false;
  int port = 54000;
  scale = 1.0;
  int jpegfactor = 70;
  mon = NULL;
  im.width = im.height = 0;
  im.size = 0; im.data = 0; im.imageFormat = urbi::IMAGE_RGB;
  
  int argp = 1;
  while (argp <argc) 
  {
    const std::string arg(argv[argp]);
    if (arg == "-p") 
      port = strtol(argv[++argp],0,0);
    else if (arg == "-j") 
      jpegfactor = strtol(argv[++argp],0,0);
    else if (arg == "-d")
      device = argv[++argp];
    else if (arg == "-f")
      frequency = strtol(argv[++argp],0,0);
    else if (arg == "-s")
      sscanf(argv[++argp],"%f",&scale);
    else if (arg == "-r")
      reconstruct = true;
    else if (arg == "-o")
      fileName = argv[++argp];
    else if (arg == "-h") 
    {
      usage(argv[0]);
      exit(0);
    }
    else break;
    ++argp;
  }
  if (argc-argp != 2)
  {
    std::cerr << argc << " " << argp << std::endl;
    usage(argv[0]);
    urbi::exit(1);
  }


  urbi::USyncClient client (argv[argp+1], port);
  if (client.error())
    urbi::exit(1);

  client.setCallback(showImage, "uimg");

  client.send("%s.resolution  = 0;", device);
  client.send("%s.jpegfactor = %d;", device, jpegfactor);

  client << "%s.reconstruct = " << (reconstruct ? 1 : 0)
	 << urbi::semicolon;

  if (!fileName)
  {
    imcount = 0;
    format = (argv[argp][0]=='r')?0:1;
    client.send("%s.format = %d;", device, format);
    if (!frequency)
      client.send("loop {uimg << %s.val; noop},",device);
    else 
      client.send("every (%d) uimg << %s.val,",frequency, device);
    urbi::execute();
  }
  else
  {
    /* Use syncGetImage to save one image to a file. */
    char buff[1000000];
    int sz = 1000000;
    int w, h;
    switch (argv[argp][0])
    {
      case 'r':
	format = urbi::IMAGE_RGB;
	break;
      case 'y':
	format = urbi::IMAGE_YCbCr;
	break;
      case 'p':
	format = urbi::IMAGE_PPM;
	break;
      case 'j':
	format = urbi::IMAGE_JPEG;
	break;
      default:
	std::cerr << "invalid format "<<argv[argp]<<std::endl;
	usage(argv[0]);
	exit(1);
    };

    client.syncGetImage(device, buff, sz,
			format,
			(format == urbi::IMAGE_JPEG
			 ? urbi::URBI_TRANSMIT_JPEG
			 : urbi::URBI_TRANSMIT_YCbCr),
			w, h);

    FILE *f = fopen(fileName, "w");

    if (!f)
      urbi::exit(2);

    fwrite(buff, 1, sz, f);
    fclose(f);
    exit(0);
  }

  return 0;
}
