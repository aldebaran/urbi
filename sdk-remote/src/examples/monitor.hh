/*
 * Copyright (C) 2005-2006, 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef _MONITOR_HH
# define _MONITOR_HH

# ifdef WIN32
#  include "monitor-win.hh"
# else
#  include "libport/cstring"
#  include <libport/cerrno>
#  include <libport/cassert>
#  include <iostream>
#  include <X11/X.h>
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#  include <list>
#  include <sys/ipc.h>
#  include <sys/shm.h>
#  include <X11/extensions/XShm.h>

typedef unsigned char bits8;

class Monitor
{
 public:
  // if fastmode enabled, use its own display: better framerate, but
  // irresponsive if not frequently updated
  Monitor(int, int, const char * name=NULL, bool fastMode = true);
  ~Monitor();

  void createWindow(const char* name);
  int  VisualClass ()
  {
    return visual->c_class;
  }
  int  Depth ()
  {
    return depth;
  }
  int  Width ()
  {
    return xImage != NULL ? xImage->width : 0;
  }
  int  Height ()
  {
    return xImage != NULL ? xImage->height : 0;
  }
  XImage *X ()
  {
    return xImage;
  }
  Bool IsShared ()
  {
    return isShared;
  }
  Bool HasSharedPixmap ()
  {
    return (Bool) (sharedPixmap != None);
  }
  Pixmap SharedPixmap ()
  {
    return sharedPixmap;
  }
  void  setName (const char * name)
  {
    XStoreName(display, window, name);
  }
  int   setImage(bits8*, int);
  int   createImage();
  int   destroyImage();
  void  clear();
  int   put();

  static void       processMessages();

 private:
  Window          window;

  Visual          *visual;
  int             depth;
  Bool            isShared; // MITSHM
  XImage          *xImage;
  Pixmap          sharedPixmap; // None (0L) if unassigned
  XShmSegmentInfo shmInfo;

  int	               screenNumber, windowsHeight, windowsWidth;
  Screen              *screen;
  XWindowAttributes   windowAttributes;
  int                 x, y, w, h;

  XEvent              event;
  Atom                atomWMDeleteWindow;
  GC                  gc;
  Display             *localDisplay;

  static pthread_mutex_t      lock;

  static Display    *display;  //shared display
  static void addList(Monitor *);
  static void removeList(Monitor *);
  static std::list<Monitor*> monitorList;
};

# endif

#endif
