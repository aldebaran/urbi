/*
 * Copyright (C) 2005-2011, Gostai S.A.S.
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

#include <kernel/config.h>
#include <libport/cli.hh>
#include <libport/cstdio>
#include <libport/ctime>
#include <libport/package-info.hh>
#include <libport/sys/types.h>
#include <libport/sys/stat.h>
#include <libport/sysexits.hh>

#include <libport/fcntl.h>
#include <urbi/uclient.hh>
#include <urbi/uconversion.hh>
#include <urbi/package-info.hh>

#ifndef WIN32
# include <sys/ioctl.h>
# ifdef HAVE_SYS_SOUNDCARD_H
#   include <sys/soundcard.h>
# endif
#endif

static const char *dsp = "/dev/dsp";

using libport::program_name;

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


FILE* file;
bool out_to_dsp;
bool with_header;
bool was_with_header;
int totallength;

namespace
{

  static
  void
  error(int status, const std::string& msg)
  {
    std::cerr << libport::program_name() << ": " << msg << std::endl;
    exit(status);
  }

#define FRAISE(Status, ...)                      \
  error(Status, libport::format(__VA_ARGS__))


  static
  void
  usage(libport::OptionParser& parser)
  {
    std::cout <<
      "usage: " << program_name() << " [options]\n"
      "    record and play (or save) sound from a robot\n";
    parser.options_doc(std::cout);
    ::exit(EX_OK);
  }

  static
  void
  version()
  {
    std::cout << "urbi-sound" << std::endl
              << urbi::package_info() << std::endl
              << libport::exit(EX_OK);
  }
}

static urbi::UCallbackAction
endProgram(const urbi::UMessage&)
{
  if (was_with_header)
  {
    //fclose(file);
    //file = fopen(fname, "w");
    // seek to set correct size in wav header
    if (fseek(file, offsetof(wavheader, length), SEEK_SET) == -1)
      FRAISE(1, "cannot seek output file (stdout?)");
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
  static bool initialized = false;
  if (!initialized)
  {
    initialized = true;
    totallength = 0;
    out.data = 0;
    out.size = 0;
    if (out_to_dsp)
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

  if (msg.type == urbi::MESSAGE_DATA
      && msg.value->type == urbi::DATA_BINARY
      && msg.value->binary->type == urbi::BINARY_SOUND)
  {
    out.soundFormat = with_header? urbi::SOUND_WAV : urbi::SOUND_RAW;
    with_header = false;
    convert(msg.value->binary->sound, out);
    totallength += out.size;
    ignore = fwrite(out.data, out.size, 1, file);
  }
  return urbi::URBI_CONTINUE;
}


int main(int argc, char *argv[])
{
  libport::program_initialize(argc, argv);

  libport::OptionValue
    arg_dev("query sound on DEVICE.val (default: micro)",
            "device", 'd', "DEVICE"),
    arg_duration("recording duration in seconds",
                 "duration", 'D', "DURATION"),
    arg_out(libport::format("save sound in FILE (%s)", dsp),
            "output", 'o', "FILE");
  libport::OptionFlag
    arg_no_headers("don't include the headers when saving the sound",
                   "no-headers", 'n');

  libport::OptionParser opt_parser;
  opt_parser << "Options:"
	     << libport::opts::help
	     << libport::opts::version
	     << libport::opts::host
	     << libport::opts::port
	     << libport::opts::port_file
             << arg_dev
             << arg_duration
             << arg_out
             << arg_no_headers;

  opt_parser(libport::program_arguments());

  if (libport::opts::help.get())
    usage(opt_parser);
  if (libport::opts::version.get())
    version();

  //16000 1 16
  std::string output = arg_out.value(dsp);
  out_to_dsp = output == dsp;
  if (out_to_dsp)
  {
#ifdef HAVE_SYS_SOUNDCARD_H
    with_header = false;
    file = fopen(dsp, "wb");
    if (!file)
      FRAISE(1, "error opening device %s: %s", dsp, strerror(errno));

    int f = fileno(file);
# define IOCTL(Name, Key, Param)                        \
    do {                                                \
      int param = Param;                                \
      if (ioctl(f, SNDCTL_DSP_ ## Key, &param) == -1)   \
        FRAISE(1, "failed to set %s for %s: %s",         \
              dsp, Name, strerror(errno));              \
    } while (false)

    IOCTL("sample size", SAMPLESIZE, 16);
    IOCTL("stereo", STEREO, 1);
    IOCTL("speed", SPEED, 16000);
# undef IOCTL
#else
    error(1, "output to soundcard not supported on this computer");
#endif
  }
  else if (output == "-")
  {
    file = stdout;
  }
  else
  {
    file = fopen(output.c_str(), "wb+");
    if (!file)
      FRAISE(2, "error creating file %s: %s", output, strerror(errno));
  }

  with_header = !arg_no_headers.get();
  was_with_header = with_header;
  /// Server port.
  int port = libport::opts::port.get<int>(urbi::UClient::URBI_PORT);
  if (libport::opts::port_file.filled())
    port = libport::file_contents_get<int>(libport::opts::port_file.value());

  urbi::UClient
    client(libport::opts::host.value(urbi::UClient::default_host()),
           port);
  if (client.error())
    FRAISE(1, "client failed to set up");

  client.setCallback(getSound, "usound");
  client.setCallback(endProgram, "end");
  std::string command = SYNCLINE_WRAP
    ("var end = Channel.new(\"end\")|;\n"
     "var usound = Channel.new(\"usound\")|;\n"
     "micro.&val.notifyChange(closure() { usound << %s.val })|;\n"
     "{ sleep(%d); end << 1 },\n",
     arg_dev.value("micro"),
     arg_duration.get<float>(1.));
  client.send(command);
  urbi::execute();
}
