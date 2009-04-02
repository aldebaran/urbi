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
#include <libport/option-parser.hh>

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
usage(libport::OptionParser& parser)
{
  std::cout <<
    "usage: " << program_name() << " [options]\n"
    "Display images from an urbi server, or save one image if -o is given\n";
  parser.options_doc(std::cout);
  std::cout << std::endl <<
    "transfer Format : jpeg=transfer jpeg, raw=transfer raw\n"
    "save     Format : rgb , ycrcb, jpeg, ppm"
              << std::endl;
  ::exit(EX_OK);
}

int
main (int argc, char *argv[])
{
  libport::program_initialize(argc, argv);
  signal(SIGINT, closeandquit);
  const char* device;
  std::string arg_format;
  const char* fileName = 0;
  mon = NULL;
  im.width = im.height = 0;
  im.size = 0;
  im.data = 0;
  im.imageFormat = urbi::IMAGE_RGB;

  libport::OptionValue
    arg_period("query images at given period (in milliseconds)",
               "period", 'p', "PERIOD"),
    arg_jpeg("jpeg compression factor (from 0 to 100, def 70)",
             "jpeg", 'j', "FACTOR"),
    arg_dev("query image on DEVICE.val (default: camera.val)",
            "device", 'd', "DEVICE"),
    arg_out("query and save one image to FILE",
            "output", 'o', "FILE"),
    arg_scale("rescale image with given FACTOR (display only)",
              "scale", 's', "FACTOR"),
    arg_form("select format of the image (rgb, ycrcb, jpeg, ppm)",
             "format", 'F', "FORMAT");
  libport::OptionFlag
    arg_rec("use reconstruct mode (for aibo)",
            "reconstruct", 'r');

  libport::OptionParser opt_parser;
  opt_parser << "Options:"
	     << libport::opts::help
	     << libport::opts::host
	     << libport::opts::port
	     << arg_period
	     << arg_form
	     << arg_rec
	     << arg_jpeg
	     << arg_dev
	     << arg_out
	     << arg_scale;

  opt_parser(libport::program_arguments());

  if (libport::opts::help.get())
    usage(opt_parser);
  device = arg_dev.value("camera").c_str();
  if (arg_form.filled())
    arg_format = arg_form.value();
  scale = arg_scale.get<float>(1.0);
  if (arg_out.filled())
    fileName = arg_out.value().c_str();

  urbi::USyncClient client(libport::opts::host.value("localhost"),
			   libport::opts::port.get<int>(urbi::UClient::URBI_PORT));
  if (client.error())
    std::cerr << libport::program_name() << ": client failed to set up"
	      << std::endl
              << libport::exit(1);

  client.setCallback(showImage, "uimg");

  client.send("%s.resolution  = 0;", device);
  client.send("%s.jpegfactor = %d;", device, arg_jpeg.get<int>(70));

  client << device << ".reconstruct = " << (arg_rec.get() ? 1 : 0)
	 << urbi::semicolon;

  if (!fileName)
  {
    imcount = 0;
    format = (arg_format[0] == 'r') ? 0 : 1;
    client.send("%s.format = %d;", device, format);
    if (int period = arg_period.get<int>(0))
      client.send("every (%dms) uimg << %s.val,", period, device);
    else
      client.send("loop { uimg << %s.val; noop },", device);
    urbi::execute();
  }
  else
  {
    switch (arg_format[0])
    {
      case 'r':	format = urbi::IMAGE_RGB;	break;
      case 'y':	format = urbi::IMAGE_YCbCr;	break;
      case 'p':	format = urbi::IMAGE_PPM;	break;
      case 'j':	format = urbi::IMAGE_JPEG;	break;
      default:
	std::cerr << program_name() << ": invalid format :"
                  << arg_format << std::endl
                  << libport::exit(EX_USAGE);
    };
    /* Use syncGetImage to save one image to a file. */
    char buff[1000000];
    size_t sz = sizeof buff;
    size_t w, h;
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
