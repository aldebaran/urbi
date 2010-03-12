/*
 * Copyright (C) 2004, 2006-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \brief Sample image acquisition urbi client.

/* This simple demonstration program display or save images from an
 * Urbi server.
 */

#include <libport/cstdio>
#include <libport/csignal>

#include <libport/cli.hh>
#include <libport/program-name.hh>
#include <libport/option-parser.hh>
#include <libport/sysexits.hh>

#include <urbi/uconversion.hh>
#include <urbi/usyncclient.hh>

#include "monitor.hh"

using libport::program_name;

size_t imcount;
Monitor* mon = 0;
float scale;
urbi::UImage im;

/* Our callback function */
namespace
{
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
    /* Calculate framerate. */
    if (!(imcount % 20))
      {
        static int tme = 0;
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

  static
  int
  format(const std::string& s)
  {
    switch (s[0])
    {
    case 'r': return urbi::IMAGE_RGB;
    case 'y': return urbi::IMAGE_YCbCr;
    case 'p': return urbi::IMAGE_PPM;
    case 'j': return urbi::IMAGE_JPEG;
    default:
      std::cerr << program_name() << ": invalid format :"
                << s << std::endl
                << libport::exit(EX_USAGE);
    };
  }
}


int
main (int argc, char *argv[])
{
  libport::program_initialize(argc, argv);
  signal(SIGINT, closeandquit);
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
             "format", 'F', "FORMAT"),
    arg_resolution("select resolution of the image (0=biggest)",
                   "resolution", 'R', "RESOLUTION");
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
	     << arg_resolution
	     << arg_scale;

  opt_parser(libport::program_arguments());

  if (libport::opts::help.get())
    usage(opt_parser);
  const char* device = arg_dev.value("camera").c_str();

  std::string arg_format;
  if (arg_form.filled())
    arg_format = arg_form.value();
  scale = arg_scale.get<float>(1.0);
  if (arg_out.filled())
    fileName = arg_out.value().c_str();

  urbi::USyncClient
    client(libport::opts::host.value(urbi::UClient::default_host()),
           libport::opts::port.get<int>(urbi::UClient::URBI_PORT));
  if (client.error())
    std::cerr << libport::program_name() << ": client failed to set up"
	      << std::endl
              << libport::exit(1);

  if (1 < client.kernelMajor())
    client.send("uimg = Channel.new(\"uimg\")|;");
  client.setCallback(showImage, "uimg");

  client.send("%s.resolution  = %s;",
              device, arg_resolution.value("0").c_str());
  client.send("%s.jpegfactor = %d;",
              device, arg_jpeg.get<int>(70));

  client << device << ".reconstruct = " << (arg_rec.get() ? 1 : 0)
	 << urbi::semicolon;

  if (fileName)
  {
    int fmt = format(arg_format);
    /* Use syncGetImage to save one image to a file. */
    char buff[1000000];
    size_t sz = sizeof buff;
    size_t w, h;
    client.syncGetImage(device, buff, sz,
			fmt,
			(fmt == urbi::IMAGE_JPEG
			 ? urbi::URBI_TRANSMIT_JPEG
			 : urbi::URBI_TRANSMIT_YCbCr),
			w, h);

    if (FILE *f = fopen(fileName, "w"))
    {
      // FIXME: Don't ignore return value.
      (void) fwrite(buff, 1, sz, f);
      fclose(f);
    }
    else
      std::cerr << program_name() << ": cannot create file " << fileName
                << ": " << strerror(errno)
                << libport::exit(EX_OSERR);
  }
  else
  {
    imcount = 0;
    int fmt = (arg_format[0] == 'r') ? 0 : 1;
    client.send("%s.format = %d;", device, fmt);
    client.waitForKernelVersion(true);
    if (int period = arg_period.get<int>(0))
      client.send("every (%dms) uimg << %s.val,", period, device);
    else if (1 < client.kernelMajor())
      client.send("var handle = WeakPointer.new;\n"
                  "%s.getSlot(\"val\").notifyChange(handle, closure() {\n"
                  "  connectionTag:\n"
                  "    this.send(%s.val.asString, \"uimg\")\n"
                  "});",
                  device, device);
    else
      client.send("loop { uimg << %s.val; noop },", device);
    urbi::execute();
  }


  return 0;
}
