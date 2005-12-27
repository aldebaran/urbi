#include <uclient.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#ifdef WIN32
#define usleep(a) Sleep(a/1000)
#endif 

bool over=false;
static int totalsize=0;
static int starttime=0;
UCallbackAction bw(const UMessage &msg) {




  totalsize+=msg.image.size+strlen(msg.tag)+20; //aproximately, but since bsz is more or less 400k...

  if (!strcmp(msg.tag,"be")) {
	msg.client.printf("received %d bytes in %d miliseconds: bandwidth is %d bytes per second.\n",
		   totalsize,
		   msg.client.getCurrentTime()-starttime,
		   totalsize*1000/(msg.client.getCurrentTime()-starttime));

	over=true;
  }

  return URBI_CONTINUE;
}

int main(int argc, char * argv[]) {

 if (argc<2) {
   printf("usage: %s robot\n",argv[0]); exit(1);
 }

 UClient &c= * new UClient(argv[1]);

 if (c.error()) exit(1);

 c.printf("requesting raw images from server to test bandwidth...\n");
 
 c.setCallback(bw,"bw");
 c.setCallback(bw,"be");

 c << "camera.format = 0;camera.resolution = 0;noop;noop;";
 
 starttime=c.getCurrentTime();
 c << " for (i=0;i<9;i++) bw:camera.val|";
 c << "be:camera.val;";
 urbi::execute();
}
