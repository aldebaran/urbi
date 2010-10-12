/*
 * Copyright (C) 2005-2006, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef MONITOR_WIN_HH
# define MONITOR_WIN_HH

typedef unsigned char bits8;

# include "libport/windows.hh"

class Monitor
{

 public:
  Monitor(int, int, const char * name=NULL);
  ~Monitor();

  int setImage(bits8*, int);

 private:
  int w, h;
  HWND window;
};
#endif // !MONITOR_WIN_HH
