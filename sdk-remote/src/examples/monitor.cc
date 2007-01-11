/*******************************************************************************

	   filename             : monitor.cpp
	   description          : Class implementation

	   copyright            : (C) 2004, 2006 by Jean-Christophe Baillie
	   email                : jean-christophe.baillie@m4x.org

********************************************************************************/

#include "libport/cstdio"
#include <cstdlib>
#include <pthread.h>
#include "libport/windows.hh"

#include "monitor.h"

#ifdef WIN32
# include "monitor-win.cpp"
#else


static char *AtomWMDeleteWindowName = (char *) "WM_DELETE_WINDOW";


Display    *Monitor::display;
std::list<Monitor*> Monitor::monitorList;
pthread_mutex_t      Monitor::lock=PTHREAD_MUTEX_INITIALIZER;

static void *
wrapper(void *)
{
  Monitor::processMessages();
  return 0;
}

void
Monitor::addList(Monitor *mon)
{
  if (monitorList.empty())
    {
      display = NULL;
      mon->gc = 0;

      // Open display
      if ((display = XOpenDisplay(NULL)) == NULL)
	{
	  // NULL for DISPLAY
	  printf("Error: XOpenDisplay() failed\n");
	  exit(1);
	}

      mon->gc = DefaultGC(display, DefaultScreen(display));
      XSetForeground(display, mon->gc, 255 + 256 * 255 + 256 * 256 * 255);
      pthread_t *pt=new pthread_t;
      monitorList.push_back(mon);

      pthread_create(pt, 0, &wrapper, 0);
    }
  else
    monitorList.push_back(mon);
}


void
Monitor::removeList(Monitor * mon)
{
  monitorList.remove(mon);
  XFreeGC(display, mon->gc);
  if (monitorList.empty() && display != NULL)
    {
      XCloseDisplay(display);
      display = NULL;
    }
}


void
Monitor::processMessages()
{
  XEvent event;
  //plan b
  printf("Processmessages spawned\n");
  while (!monitorList.empty())
    {
      pthread_mutex_lock(&lock);
      while (XPending(display) > 0)
	XNextEvent(display, &event);
      for (std::list<Monitor *>::iterator it=monitorList.begin();
	   it != monitorList.end(); ++it)
	(*it)->put();
      pthread_mutex_unlock(&lock);
      usleep(300000);
    }
  return;

  while (!monitorList.empty())
    {
      bool found = false;
      XNextEvent(display, &event);
      switch (event.type)
	{
	case Expose:
	  if (event.xexpose.count == 0)
	    {
	      for (std::list<Monitor *>::iterator it=monitorList.begin();
		   it != monitorList.end(); ++it)
		if ((*it)->window == event.xexpose.window)
		  {
		    (*it)->put();
		    // FIXME: What can be done here instead of these
		    // two casts?
		    printf("repainting %d\n",
			   static_cast<int>(event.xexpose.window));
		    found = true;
		    break;
		  }
	      if (!found)
		printf("error: expose event for unknown window %d\n",
		       static_cast<int>(event.xexpose.window));
	    }
	}
    }
}

/*-----------------------------------------------------------------------------*/

inline void
setimageat(bits8 ** point, bits8 a, bits8 b, bits8 c)
{
  (*point)[0] = a;
  (*point)[1] = b;
  (*point)[2] = c;
  (*point)[3] = c;

  *point = *point + 4;
}

Monitor::Monitor(int _w, int _h, const char * name, bool _fastMode)
  : sharedPixmap(None)
{
  isShared = False;
  xImage = NULL;

  w = _w;
  h = _h;
  if (! _fastMode)
    {
      addList(this);
      localDisplay = display;
    }
  else
    {
      // Open display

      if ((localDisplay = XOpenDisplay(NULL)) == NULL)
	{
	  // NULL for DISPLAY
	  printf("Error: XOpenDisplay() failed\n");
	  exit(1);
	}
      gc = DefaultGC(localDisplay, DefaultScreen(localDisplay));
      XSetForeground(localDisplay, gc, 255 + 256 * 255 + 256 * 256 * 255);
    }

  // Obtain WM protocols atom for ClientMessage exit event
  pthread_mutex_lock(&lock);
  atomWMDeleteWindow = XInternAtom(localDisplay, AtomWMDeleteWindowName, True);
  if (atomWMDeleteWindow == None)
    {
      printf("Error: %s atom does not exist\n", AtomWMDeleteWindowName);
      exit(1);
    }

  createWindow(name ? name : "XImage - XShm optimized");
  pthread_mutex_unlock(&lock);
  printf("Monitor created window %d\n", static_cast<int>(window));
}

Monitor::~Monitor()
{
  removeList(this);
  XFreeGC(localDisplay, gc);
  if (display != localDisplay)
    XCloseDisplay(localDisplay);

  destroyImage();
}

int Monitor::createImage()
{
  XGCValues gcValues;
  unsigned long gcValuesMask;
  XWindowAttributes windowAttributes;


  gcValues.function = GXcopy;
  gcValuesMask = GCFunction;
  gc = XCreateGC(localDisplay, window, gcValuesMask, &gcValues);

  XGetWindowAttributes(localDisplay, window, &windowAttributes);

  visual = windowAttributes.visual;
  depth = windowAttributes.depth;
  isShared = XShmQueryExtension(localDisplay);
  char * D = getenv("DISPLAY");
  if (D)
    {
      D=strdup(D);
      char * delim=strstr(D, ":");
      if (delim)
	*delim=0;
      if (D[0]!=0 && strcmp(D, "localhost") && strcmp(D, "127.0.0.1"))
	isShared = false;
      free(D);
    }
  if (getenv("DISABLE_SHM") != 0)
    isShared = false;
  try {
    errno = 0;
    xImage = NULL;
    sharedPixmap = None;
    if (isShared)
    {
      shmInfo.shmid = -1;
      shmInfo.shmaddr = NULL;
      if ((xImage = XShmCreateImage(localDisplay, visual, depth, ZPixmap,
				    NULL, &shmInfo, w, h)) == NULL)
      {
	throw ("XShmCreateImage");
      }
      if ((shmInfo.shmid = shmget(IPC_PRIVATE, xImage->bytes_per_line *
				  xImage->height, IPC_CREAT | 0777)) < 0)
      {
	// Create segment
	throw ("shmget");
      }
      if ((shmInfo.shmaddr = (char *) shmat(shmInfo.shmid, 0, 0)) < 0)
      {
	// We attach
	shmInfo.shmaddr = NULL;
	throw ("shmat");
      }
      xImage->data = shmInfo.shmaddr;
      shmInfo.readOnly = False;
      if (!XShmAttach(localDisplay, &shmInfo))
      {
	// X attaches
	throw ("XShmAttach");
      }


      XSync(localDisplay, False);


      if (XShmPixmapFormat(localDisplay) == ZPixmap)
      {
	if ((sharedPixmap = XShmCreatePixmap(localDisplay, window,
					     shmInfo.shmaddr, &shmInfo, w, h,
					     depth)) == None)
	{
	  ;	// HasSharedPixmap() will return false.
	}
      }
    }
    else
    {
      if ((xImage = XCreateImage(localDisplay, visual, depth, ZPixmap, 0,
				 NULL, w, h, 16, 0)) == NULL)
      {
	throw ("XCreateImage");
      }
      if ((xImage->data = static_cast<char *> (malloc (xImage->bytes_per_line
						       * xImage->height)))
	   == NULL)
      {
	throw ("malloc");
      }
    }
    return 0;
  }
  catch(char *function)
  {
    printf("%s%s:%s\n", "Error: Image::Create failed in ",
	   function, ((errno == 0) ? "No further info" : strerror(errno)));

    destroyImage();
    return -1;
  }

}

/*------------------------------------------------------------------------*/

int Monitor::destroyImage()
{
  if (xImage == NULL)
    return 0;	// Nothing to do

  if (isShared)
    {
      if (shmInfo.shmid >= 0)
	{
	  XShmDetach(localDisplay, &shmInfo);	// X detaches
	  shmdt(shmInfo.shmaddr);	// We detach
	  shmInfo.shmaddr = NULL;
	  shmctl(shmInfo.shmid, IPC_RMID, 0);	// Destroy segment
	  shmInfo.shmid = -1;
	}
    }
  else if (xImage->data != NULL)
    free(xImage->data);

  xImage->data = NULL;

  XDestroyImage(xImage);

  xImage = NULL;

  if (sharedPixmap != None)
    {
      XFreePixmap(localDisplay, sharedPixmap);
      sharedPixmap = None;
    }

  return 0;
}


void
Monitor::clear()
{
  if (xImage == NULL)
    return;

  memset(xImage->data, 0, xImage->height * xImage->bytes_per_line);
}


int
Monitor::put()
{
  if (xImage == NULL)
    return -1;

  int width = Width();
  int height = Height();

  if (isShared)
    XShmPutImage(localDisplay, window, gc, xImage,
		 0, 0, 0, 0, width, height, False);
  else
    XPutImage(localDisplay, window, gc, xImage,
	      0, 0, 0, 0, width, height);

  return 0;
}

int
Monitor::setImage(bits8* buffer, int bufferlen)
{
  if (xImage == NULL)
    return -1;
  //XImage *xImage;
  bits8* imageLine;
  int i = 0;

  if (localDisplay != display)
  {
    if (XPending(localDisplay) > 0)
    {
      XNextEvent(localDisplay, &event);
      switch (event.type)
      {
	case ClientMessage:
	  if ((int) event.xclient.data.l[0] == (int) atomWMDeleteWindow)
	    return -1;
	  break;

	case Expose:
	  if (event.xexpose.count == 0)
	    put();

	  break;
      }
    }
    else
    {
      //xImage = X();
      imageLine = (bits8 *) xImage->data;
      for (i = 0; i < bufferlen / 3; ++i)
	setimageat(&imageLine,
		   *(buffer + i * 3 + 2),
		   *(buffer + i * 3 + 1),
		   *(buffer + i * 3 + 0));
      put();
    }
  }
  else
  {
    imageLine = (bits8 *) xImage->data;
    for (i = 0; i < bufferlen / 3; ++i)
      setimageat(&imageLine,
		 *(buffer + i * 3 + 2),
		 *(buffer + i * 3 + 1),
		 *(buffer + i * 3 + 0));
  }
  return 1; // ?
}

/**************************************************************************/

void
Monitor::createWindow(const char *name)
{
  screenNumber = DefaultScreen(localDisplay);
  screen = XScreenOfDisplay(localDisplay, screenNumber);
  windowsHeight = h;
  windowsWidth = w;

  window = XCreateSimpleWindow(localDisplay, RootWindowOfScreen(screen),
			       100, 100, w, h, 0,
			       BlackPixelOfScreen(screen),
			       BlackPixelOfScreen(screen));

  XStoreName(localDisplay, window, name);
  XGetWindowAttributes(localDisplay, window, &windowAttributes);

  if (((windowAttributes.depth == 8)
       && (windowAttributes.visual->c_class != PseudoColor))
      || ((windowAttributes.depth > 8)
	  && (windowAttributes.visual->c_class != TrueColor)))
    {
      printf("Error: Visual not supported\n");
      exit(1);
    }

  // Create PseudoColor HI240 colormap, if needed

  if (windowAttributes.depth == 8)
    {
      printf("Error : display must be 32bits depth");
      exit(1);
    }


  // Create image

  if (createImage() < 0)
    {
      printf("Error: image.Create() failed\n");
      exit(1);
    }

  clear();

  // Ready to start: Display window, select events, and initiate
  // capture sequence
  XMapRaised(localDisplay, window);
  XSetWMProtocols(localDisplay, window, &atomWMDeleteWindow, 1);
  XSelectInput(localDisplay, window, StructureNotifyMask | ExposureMask);
}

#endif // !WIN32
