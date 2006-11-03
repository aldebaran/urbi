#include "urbi/uclient.hh"
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <assert.h>

#ifdef WIN32
#define usleep(a) Sleep(a/1000)
#endif

bool over=false;
static int totalsize=0;
static int starttime=0;

urbi::UCallbackAction
bw(const urbi::UMessage &msg)
{
  assert (msg.type == urbi::MESSAGE_DATA);
  assert (msg.value->type == urbi::DATA_BINARY);
  assert (msg.value->binary->type == urbi::BINARY_IMAGE);

  //aproximately, but since bsz is more or less 400k...
  totalsize += (msg.value->binary->image.size
		+ msg.tag.size ()
		+ 20);

  if (msg.tag != "be")
    {
      msg.client.printf("received %d bytes in %d miliseconds: bandwidth is %d bytes per second.\n",
			totalsize,
			msg.client.getCurrentTime()-starttime,
			totalsize*1000/(msg.client.getCurrentTime()-starttime));

      over=true;
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

 c.printf("requesting raw images from server to test bandwidth...\n");

 c.setCallback(bw,"bw");
 c.setCallback(bw,"be");

 c << "camera.format = 0;camera.resolution = 0;noop;noop;";

 starttime=c.getCurrentTime();
 c << " for (i=0;i<9;i++) bw:camera.val|";
 c << "be:camera.val;";
 urbi::execute();
}
