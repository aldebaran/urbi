typedef unsigned char bits8;

#include <windows.h>

class Monitor
{

 public:
  Monitor(int,int, const char * name=NULL);
  ~Monitor();

  int   setImage(bits8*,int);

 private:
  int w,h;
  HWND window;
};
