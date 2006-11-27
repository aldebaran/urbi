#ifndef MONITOR_WIN_H_
# define MONITOR_WIN_H_

typedef unsigned char bits8;

# ifndef _WIN32_WINNT
#  define _WIN32_WINNT 0x0400
# endif
# include <windows.h>

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
#endif /* !MONITOR_WIN_H_ */
