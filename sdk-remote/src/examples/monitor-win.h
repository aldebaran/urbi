#ifndef MONITOR_WIN_H_
# define MONITOR_WIN_H_

typedef unsigned char bits8;

# include "libport/windows.hh"

class Monitor
{

 public:
  Monitor(int, int, const char * name=NULL);
  ~Monitor();

  int   setImage(bits8*, int);

 private:
  int w, h;
  HWND window;
};
#endif /* !MONITOR_WIN_H_ */
