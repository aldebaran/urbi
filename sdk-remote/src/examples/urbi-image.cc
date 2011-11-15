/*
 * Copyright (C) 2005-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \brief Sample image acquisition urbi client.

#include <libport/cstdio>

#include <libport/cli.hh>
#include <libport/debug.hh>
#include <libport/package-info.hh>
#include <libport/program-name.hh>
#include <libport/option-parser.hh>
#include <libport/sysexits.hh>

#include <liburbi/compatibility.hh>
#include <urbi/package-info.hh>
#include <urbi/uconversion.hh>
#include <urbi/usyncclient.hh>

#include "monitor.hh"

GD_CATEGORY(UrbiImage);

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
    bool ko = msg.type != urbi::MESSAGE_DATA
        || msg.value->type != urbi::DATA_BINARY
        || msg.value->binary->type != urbi::BINARY_IMAGE;
    GD_FINFO_DUMP("Message on img channel, type %s, ok %s", msg.type, !ko);
    if (msg.value)
      GD_FINFO_DUMP("vtype %s btype %s", msg.value->type,
                    msg.value->binary->type);
    if (ko)
      return urbi::URBI_CONTINUE;

    urbi::UImage& img = msg.value->binary->image;
    im.width = static_cast<int>(static_cast<float>(img.width) * scale);
    im.height = static_cast<int>(static_cast<float>(img.height) * scale);
    convert(img, im);

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

    size_t sz = 3*im.width*im.height;
    mon->setImage((bits8 *) im.data, sz);
    ++imcount;
    return urbi::URBI_CONTINUE;
  }

  static
  void
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
  void
  version()
  {
    std::cout << "urbi-image" << std::endl
              << urbi::package_info() << std::endl
              << libport::exit(EX_OK);
  }

  /// Decode the image format string \s s.
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
      std::cerr << program_name() << ": invalid format: "
                << s << std::endl
                << libport::exit(EX_USAGE);
    }
  }
}

GD_INIT();

int
main (int argc, char *argv[])
{
  libport::program_initialize(argc, argv);
  mon = 0;
  im.width = im.height = 0;
  im.size = 0;
  im.data = 0;
  im.imageFormat = urbi::IMAGE_RGB;

  libport::OptionValue
    arg_period("query images at given period (in milliseconds)",
               "period", 'p', "PERIOD"),
    arg_jpeg("jpeg compression factor (from 0 to 100, def 70)",
             "jpeg", 'j', "FACTOR"),
    arg_dev("query image on DEVICE.val (default: camera)",
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
  opt_parser
    << "Options:"
    << libport::opts::help
    << libport::opts::version
    << libport::opts::host
    << libport::opts::port
    << libport::opts::port_file
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
  if (libport::opts::version.get())
    version();

  std::string device = arg_dev.value("camera");

  std::string arg_format = arg_form.value("jpeg");
  scale = arg_scale.get<float>(1.0);

  /// Server port.
  int port = libport::opts::port.get<int>(urbi::UClient::URBI_PORT);
  if (libport::opts::port_file.filled())
    port = libport::file_contents_get<int>(libport::opts::port_file.value());
  urbi::USyncClient
    client(libport::opts::host.value(urbi::UClient::default_host()),
           port);

  if (client.error())
    std::cerr << libport::program_name() << ": client failed to set up"
	      << std::endl
              << libport::exit(1);

  if (1 < client.kernelMajor())
    client.send(SYNCLINE_WRAP("var uimg = Channel.new(\"uimg\")|;"));
  client.setCallback(showImage, "uimg");

  client.send(SYNCLINE_WRAP(
                "%s.resolution = %s|;\n"
                "%s.jpegfactor = %d|;\n"
                "%s.reconstruct = %d|;",
                device, arg_resolution.value("0"),
                device, arg_jpeg.get<int>(70),
                device, arg_rec.get() ? 1 : 0));

  if (arg_out.filled())
  {
    const std::string file = arg_out.value();
    FILE *f = fopen(file.c_str(), "w");
    if (!f)
      std::cerr << program_name() << ": cannot create file " << file
                << ": " << strerror(errno)
                << libport::exit(EX_OSERR);

    int fmt = format(arg_format);
    // Use syncGetImage to save one image to a file.
    char buff[1000000];
    size_t sz = sizeof buff;
    size_t w, h;
    client.syncGetImage(device.c_str(), buff, sz,
			fmt,
			(fmt == urbi::IMAGE_JPEG
			 ? urbi::URBI_TRANSMIT_JPEG
			 : urbi::URBI_TRANSMIT_YCbCr),
			w, h);
    size_t written = fwrite(buff, 1, sz, f);
    LIBPORT_USE(written);
    assert_eq(sz, written);
    fclose(f);
  }
  else
  {
    imcount = 0;
    int fmt = (arg_format[0] == 'r') ? 0 : 1;
    client.send(SYNCLINE_WRAP("%s.format = %d|;", device, fmt));
    client.waitForKernelVersion(true);
    std::string command;
    if (int period = arg_period.get<int>(0))
      command = SYNCLINE_WRAP("every (%dms) uimg << %s.val,",
                              period, device);
    else if (client.kernelMajor() < 2)
      command = SYNCLINE_WRAP("loop { uimg << %s.val; noop },", device);
    else
      command = SYNCLINE_WRAP
        ("%s.&val.notifyChange(closure() {\n"
         "  connectionTag:\n"
         "    this.send(%s.val.asString, \"uimg\")\n"
         "});",
         device, device);
    client.send(command);
    urbi::execute();
  }

  return 0;
}
