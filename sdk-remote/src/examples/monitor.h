/*******************************************************************************

	   filename             : monitor.h
	   description          : Class definition

	   copyright            : (C) 2004, 2006 by Jean-Christophe Baillie
	   email                : jean-christophe.baillie@m4x.org

********************************************************************************/

#ifndef _MONITOR_H
# define _MONITOR_H

# ifdef WIN32
#  include "monitor-win.h"
# else
# include <cstring>
# include <cerrno>
# include <cassert>
# include <iostream>
# include <X11/X.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <list>
# include <sys/ipc.h>
# include <sys/shm.h>
# include <X11/extensions/XShm.h>

typedef unsigned char bits8;

class Monitor
{
 public:
  // if fastmode enabled, use its own display: better framerate, but
  // irresponsive if not frequently updated
  Monitor(int,int, const char * name=NULL, bool fastMode = true);
  ~Monitor();

  void createWindow(const char* name);
  int  VisualClass() {return (visual->c_class);}
  int  Depth()  {return (depth);}
  int  Width()  {return ((xImage != NULL) ? xImage->width : 0);}
  int  Height() {return ((xImage != NULL) ? xImage->height : 0);}
  XImage *X() {return (xImage);};
  Bool IsShared() {return (isShared);}
  Bool HasSharedPixmap() {return ((Bool) (sharedPixmap != None));}
  Pixmap SharedPixmap() {return (sharedPixmap);}
  int  setName(const char * name) {  XStoreName(display, window, name);}
  int   setImage(bits8*,int);
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

  int	               screenNumber,windowsHeight,windowsWidth;
  Screen              *screen;
  XWindowAttributes   windowAttributes;
  int                 x, y,w,h;

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
