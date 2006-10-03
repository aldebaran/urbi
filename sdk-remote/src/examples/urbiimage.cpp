/****************************************************************************
 * $Id: urbiimage.cpp,v 1.13 2005/09/30 17:48:00 nottale Exp $
 *
 * Sample image acqusition urbi client.
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

/*
 * This simple demonstration program display or save images from an Urbi server.
 */

#include <cstdio>
#include <csignal>

#include "usyncclient.h"
#include "monitor.h"

int imcount;
int format;
Monitor *mon=0;
unsigned char * buffer=NULL;


/* Our callback function */
urbi::UCallbackAction
showImage(const urbi::UMessage &msg)
{
  if (msg.type != urbi::MESSAGE_DATA
      || msg.value->type != urbi::DATA_BINARY
      || msg.value->binary->type != urbi::BINARY_IMAGE)
    return urbi::URBI_CONTINUE;

  urbi::UImage& img = msg.value->binary->image;

  static unsigned char* buffer = (unsigned char*) malloc(3*400*400);
  int sz = 500000;
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
    mon = new Monitor(img.width, img.height, "image");

  switch (img.imageFormat)
    {
    case urbi::IMAGE_JPEG:
      urbi::convertJPEGtoRGB((const urbi::byte *) img.data, img.size,
			     (urbi::byte *) buffer, sz);
      break;
    case urbi::IMAGE_YCbCr:
      sz = img.size;
      urbi::convertYCrCbtoRGB((const urbi::byte *) img.data, img.size,
			      (urbi::byte *) buffer);
      break;
    }
  
  mon->setImage((bits8 *) buffer, sz);
  imcount++;
  return urbi::URBI_CONTINUE;
}

void
closeandquit (int sig)
{
  delete urbi::getDefaultClient();
  urbi::exit(0);
}


int
main (int argc, char *argv[])
{
  signal(SIGINT, closeandquit);
  mon = NULL;

  if (argc != 3)
    {
      fprintf(stderr,
	      "Missing argument\n"
	      "usage: urbiimage format robotname [reconstruct]: display images\n"
	      "\tFormat : jpeg=transfer jpeg, raw=transfer raw\n"
	      "usage: urbiimage -tofile format filename robotname [reconstruct]: save 1 image.\n"
	      "\t Format: rgb: RGB ycrcb:YCrCb jpeg: JPEG ppm:PPM\n");
      urbi::exit(1);
    }

  int mode = 0;
  if (argc >=5)
    mode = 1;
  urbi::USyncClient client (argv[2+mode*2]);

  if (client.error() != 0)
    urbi::exit(0);

  int t = client.getCurrentTime();

  client.setCallback(showImage, "uimg");

  client.send("camera.resolution  = 0;");
  client.send("camera.jpegfactor = 70;");

  client << "camera.reconstruct = " << ((argc>3+mode*2)? 1:0)
	 << urbi::semicolon;

  if (mode == 0)
    {
      imcount = 0;
      format = (argv[1][0]=='r')?0:1;
      client.send("camera.format = %d;", format);
      client.send("loop {uimg: camera.val; noop},");
      urbi::execute();
    }
  else
    {
      /* Use syncGetImage to save one image to a file. */
      char buff[1000000];
      int sz = 1000000;
      int w, h;
      switch (argv[2][0])
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
	};

      client.syncGetImage("camera", buff, sz,
			  format,
			  (format == urbi::IMAGE_JPEG
			   ? urbi::URBI_TRANSMIT_JPEG
			   : urbi::URBI_TRANSMIT_YCbCr),
			  w, h);

      FILE *f = fopen(argv[3], "w");

      if (!f)
	urbi::exit(2);

      fwrite(buff, 1, sz, f);
      fclose(f);
    }

  return 0;
}
