/****************************************************************************
 * Sample image acqusition urbi client.
 *
 * Copyright (C) 2004, 2006, 2007, 2008, 2009 Jean-Christophe Baillie.  All rights reserved.
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

#include <libport/cstdio>
#include <csignal>

#include <libport/cli.hh>
#include <libport/program-name.hh>

#include <urbi/usyncclient.hh>

#include "monitor.h"

using libport::program_name;

int imcount;
int format;
Monitor* mon = 0;
unsigned char* buffer=NULL;
float scale;
urbi::UImage im;

/* Our callback function */
static
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

  size_t sz = 3*im.width*im.height;
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

static void
closeandquit (int)
{
  delete urbi::getDefaultClient();
  urbi::exit(0);
}


static void
usage()
{
  std::cout <<
    "usage: " << program_name() << " [options]\n"
    "Display images from an urbi server, or save one image if\n"
    "-o is given\n"
    "\n"
    "Options:\n"
    "  -H, --host HOST      server (robot) host name\n"
    "  -P, --port PORT      server port\n"
    "  -P, --period PERIOD  query images at given period (in milliseconds)\n"
    "  -r                   use reconstruct mode (for aibo)\n"
    "  -j factor            jpeg compression factor (from 0 to 100, def 70)\n"
    "  -d device            query image on device.val (default: camera.val)\n"
    "  -o file              query and save one image to file\n"
    "  -s scale             rescale image with given factor (display only)\n"
    "transfer Format : jpeg=transfer jpeg, raw=transfer raw\n"
    "save     Format : rgb , ycrcb, jpeg, ppm"
              << std::endl;
}

int
main (int argc, char *argv[])
{
  libport::program_initialize(argc, argv);
  signal(SIGINT, closeandquit);
  int period = 0;
  const char* device = "camera";
  std::string arg_format;
  char* fileName = 0;
  bool reconstruct = false;
  /// Server host name.
  std::string host = "localhost";
  /// Server port.
  int port = urbi::UClient::URBI_PORT;
  scale = 1.0;
  int jpegfactor = 70;
  mon = NULL;
  im.width = im.height = 0;
  im.size = 0;
  im.data = 0;
  im.imageFormat = urbi::IMAGE_RGB;

  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];
    if (arg == "-j")
      jpegfactor = strtol(argv[++i],0,0);
    else if (arg == "-d")
      device = argv[++i];
    else if (arg == "--period" || arg == "-p")
      period = strtol(argv[++i],0,0);
    else if (arg == "--format" || arg == "-f")
      arg_format = argv[++i];
    else if (arg == "--help" || arg == "-h")
      usage();
    else if (arg == "--host" || arg == "-H")
      host = argv[++i];
    else if (arg == "--port" || arg == "-P")
      port = libport::convert_argument<int> (arg, argv[++i]);
    else if (arg == "-s")
      sscanf(argv[++i], "%f", &scale);
    else if (arg == "-r")
      reconstruct = true;
    else if (arg == "-o")
      fileName = argv[++i];
    else
      libport::invalid_option(arg);
  }

  urbi::USyncClient client(host, port);
  if (client.error())
    urbi::exit(1);

  client.setCallback(showImage, "uimg");

  client.send("%s.resolution  = 0;", device);
  client.send("%s.jpegfactor = %d;", device, jpegfactor);

  client << device << ".reconstruct = " << (reconstruct ? 1 : 0)
	 << urbi::semicolon;

  if (!fileName)
  {
    imcount = 0;
    format = (arg_format[0] == 'r') ? 0 : 1;
    client.send("%s.format = %d;", device, format);
    if (!period)
      client.send("loop {uimg << %s.val; noop},", device);
    else
      client.send("every (%d) uimg << %s.val,", period, device);
    urbi::execute();
  }
  else
  {
    /* Use syncGetImage to save one image to a file. */
    char buff[1000000];
    size_t sz = sizeof buff;
    size_t w, h;
    switch (arg_format[0])
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
	std::cerr << program_name() << ": invalid format :"
                  << arg_format << std::endl
                  << libport::exit(EX_USAGE);
    };

    client.syncGetImage(device, buff, sz,
			format,
			(format == urbi::IMAGE_JPEG
			 ? urbi::URBI_TRANSMIT_JPEG
			 : urbi::URBI_TRANSMIT_YCbCr),
			w, h);

    if (FILE *f = fopen(fileName, "w"))
    {
      fwrite(buff, 1, sz, f);
      fclose(f);
    }
    else
      std::cerr << program_name() << ": cannot create file " << fileName
                << ": " << strerror(errno)
                << libport::exit(EX_OSERR);
  }

  return 0;
}
