/*
 * Copyright (C) 2005-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cassert>
#include <libport/sys/types.h>
#include <libport/sys/stat.h>
#include <libport/csignal>
#include <libport/windows.hh>

#include <urbi/uclient.hh>

bool over = false;
static int totalsize = 0;
static int starttime = 0;

static urbi::UCallbackAction
bw(const urbi::UMessage &msg)
{
  passert (msg.type,
	   msg.type == urbi::MESSAGE_DATA);
  passert (msg.value->type,
	   msg.value->type == urbi::DATA_BINARY);
  passert (msg.value->binary->type,
	   msg.value->binary->type == urbi::BINARY_IMAGE);

  //aproximately, but since bsz is more or less 400k...
  totalsize += (msg.value->binary->image.size
		+ msg.tag.size ()
		+ 20);

  if (msg.tag != "be")
  {
    msg.client.printf("received %d bytes in %d miliseconds: "
                      "bandwidth is %d bytes per second.\n",
                      totalsize,
                      msg.client.getCurrentTime()-starttime,
                      totalsize*1000/(msg.client.getCurrentTime()-starttime));

    over = true;
  }

  return urbi::URBI_CONTINUE;
}

int main(int argc, char * argv[])
{
  if (argc != 2)
  {
    printf("usage: %s robot\n", argv[0]);
    urbi::exit(1);
  }

  urbi::UClient c (argv[1]);

  if (c.error())
    urbi::exit(1);

  c.printf("Requesting raw images from server to test bandwidth...\n");

  c.setCallback(bw,"bw");
  c.setCallback(bw,"be");

  c <<
    "camera.format = 0;\n"
    "camera.resolution = 0;\n"
    "noop;\n"
    "noop;\n"
    "if (isdef(Channel))\n"
    "{\n"
    "  var Global.bw = Channel.new(\"bw\");\n"
    "  var Global.be = Channel.new(\"be\")|;\n"
    "};\n";

  starttime = c.getCurrentTime();
  c <<
    "for (var i = 0; i < 9 ; i++)\n"
    "  bw << camera.val|\n"
    "be << camera.val;\n";
  urbi::execute();
}
