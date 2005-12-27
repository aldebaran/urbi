typedef unsigned char bits8;
#include <Windows.h>
class Monitor {
 
 public:
  Monitor(int,int, const char * name=NULL);
  ~Monitor();

  int   setImage(bits8*,int);

private:
 int w,h;
	HWND window;
};