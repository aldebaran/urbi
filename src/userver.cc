/*! \file userver.cc
 *******************************************************************************

 File: userver.cc\n
 Implementation of the UServer class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */
//#define ENABLE_DEBUG_TRACES
#include <cassert>
#include <cstdlib>
#include <cstdarg>

#include <fstream>
#include <sstream>
#include <string>

#include <boost/format.hpp>
#if ! defined LIBPORT_URBI_ENV_AIBO
# include <boost/thread.hpp>
#endif

#include <libport/compiler.hh>
#include <libport/config.h>
#include <libport/containers.hh>
#include <libport/cstdio>
#include <libport/foreach.hh>
#include <libport/path.hh>
#include <libport/separator.hh>

#include "urbi/uobject.hh"
#include "urbi/usystem.hh"

#include "kernel/userver.hh"
#include "kernel/utypes.hh"

#include "ast/ast.hh"
#include "ast/nary.hh"

#include "object/atom.hh"
#include "object/primitives.hh"
#include "runner/runner.hh"
#include "scheduler/scheduler.hh"

#include "server-timer.hh"
#include "ubanner.hh"
#include "ughostconnection.hh"
#include "uobject.hh"
#include "uqueue.hh"

// Global server reference
UServer *urbiserver = 0;

const char* EXTERNAL_MESSAGE_TAG   = "__ExternalMessage__";

// Formatting for the echo and error outputs.
const char* DISPLAY_FORMAT   = "[%ld] %-35s %s";
const char* DISPLAY_FORMAT1  = "[%ld] %-35s %s : %ld";
const char* DISPLAY_FORMAT2  = "[%d] %-35s %s : %d/%d";

const char* UNKNOWN_TAG = "";
const char* MAINDEVICE  = "system";

// Memory counter system
int availableMemory;
int usedMemory;


UServer::UServer(ufloat period,
		 const char* mainName)
  : search_path(),
    scheduler_ (new scheduler::Scheduler),
    debugOutput (false),
    mainName_ (mainName),
    uservarState (false),
    stopall (false),
    period_(period),
    uid(0)
{
#if ! defined NDEBUG
  //  std::atexit(dump_timer);
  server_timer.start ();
  server_timer.dump_on_destruction (std::cerr);
  TIMER_PUSH("server");
#endif
  ::urbiserver = this;
}

UErrorValue
UServer::load_init_file(const char* fn)
{
  DEBUG (("Loading %s...", fn));
  UErrorValue res = loadFile(fn, ghost_->recv_queue_get());
  if (res == USUCCESS)
  {
    DEBUG (("done\n"));
    ghost_->new_data_added_get() = true;
  }
  else
    DEBUG (("not found\n"));
  return res;
}

void
UServer::initialize()
{
  // Set the initial time to a valid value.
  updateTime();

  // Display the banner.
  {
    bool old_debugOutput = debugOutput;
    debugOutput = true;
    display(::HEADER_BEFORE_CUSTOM);

    int i = 0;
    char customHeader[1024];
    do {
      getCustomHeader(i, customHeader, 1024);
      if (customHeader[0])
	display(customHeader);
      ++i;
    } while (customHeader[0] != 0);

    display(::HEADER_AFTER_CUSTOM);
    display("Ready.\n");

    debugOutput = old_debugOutput;
  }

  //The order is important: ghost connection, plugins, urbi.ini

  // Ghost connection
  {
    DEBUG (("Setting up ghost connection..."));
    ghost_ = new UGhostConnection(this);
    std::ostringstream o;
    o << 'U' << (long)ghost_;

    uservarState = true;
    DEBUG (("done\n"));
  }

  load_init_file("urbi/urbi.u");

  // Handle pluged UOBjects.
  // Create "uobject" in lobby where UObjects will be put.
  object::rObject uobject = object::object_class->clone();
  ghost_->lobby_->slot_set(SYMBOL(uobject), uobject);
  uobject_initialize(uobject);
  load_init_file("URBI.INI");

  // Force processing of urbi.u.
  libport::utime_t now = libport::utime();
  while (work() <= now)
    ;
}


void
UServer::main (int argc, const char* argv[])
{
  // FIXME: Save argv into Urbi world.
  (void) argc;
  (void) argv;
}

void
UServer::beforeWork ()
{
}

void
UServer::afterWork ()
{
}

libport::utime_t
UServer::work ()
{
# if ! defined LIBPORT_URBI_ENV_AIBO
  boost::recursive_mutex::scoped_lock lock(mutex);
# endif

  beforeWork ();

  work_handle_connections_ ();

  // To make sure that we get different times before and after every work
  // phase if we use a monotonic clock, update the time before and after
  // working.
  updateTime ();
  libport::utime_t next_time = scheduler_->work ();
  updateTime ();

  afterWork ();
  return next_time;
}

void
UServer::work_handle_connections_ ()
{
  // Scan currently opened connections for ongoing work
  foreach (UConnection* c, connectionList)
    if (c->active_get())
    {
      if (!c->blocked_get())
	c->continue_send();

      c->error_check_and_send(UERROR_MEMORY_OVERFLOW);
      c->error_check_and_send(UERROR_MEMORY_WARNING);
      c->error_check_and_send(UERROR_SEND_BUFFER_FULL);
      c->error_check_and_send(UERROR_RECEIVE_BUFFER_FULL);
      c->error_check_and_send(UERROR_RECEIVE_BUFFER_CORRUPTED);

      // The following code only made sense in k1, and should be
      // removed in k2, provided we are really sure it is useless.
      // The following if is also suspicous...
#if 0
      // Run the connection's command queue:
      if (c->has_pending_command ())
      {
	c->obstructed = true; // will be changed to 'false'
	{
	  //if the whole tree is visited
# if ! defined LIBPORT_URBI_ENV_AIBO
	  boost::try_mutex::scoped_lock((*c)->treeMutex);
# endif
	  c->execute((*c)->activeCommand);
	}
      }
#endif

      if (c->new_data_added_get())
      {
	// used by loadFile and eval to
	// delay the parsing after the completion
	// of execute().
	c->new_data_added_get() = false;
	c->received("");
      }
    }
}

void
UServer::work_handle_stopall_ ()
{
  if (stopall)
  {
    foreach (UConnection* c, connectionList)
      if (c->active_get() && c->has_pending_command ())
	c->drop_pending_commands ();
  }

  // Delete all connections with closing=true
  for (std::list<UConnection *>::iterator i = connectionList.begin();
       i != connectionList.end(); )
  {
    if ((*i)->closing_get())
    {
      delete *i;
      i = connectionList.erase(i);
    }
    else
      i++;
  }

  stopall = false;
}

//! UServer destructor.
UServer::~UServer()
{
}

void
UServer::vecho_key(const char* key, const char* s, va_list args)
{
  // This local declaration is rather unefficient but is necessary
  // to insure that the server could be made semi-reentrant.
  char key_[6];

  if (key == NULL)
    key_[0] = 0;
  else
  {
    strncpy(key_, key, 5);
    key_[5] = 0;
  }

  char buf1[MAXSIZE_INTERNALMESSAGE];
  vsnprintf(buf1, sizeof buf1, s, args);
  char buf2[MAXSIZE_INTERNALMESSAGE];
  snprintf(buf2, sizeof buf2, "%-90s [%5s]\n", buf1, key_);
  display(buf2);
}

void
UServer::echoKey(const char* key, const char* s, ...)
{
  va_list args;
  va_start(args, s);
  vecho_key(key, s, args);
  va_end(args);
}

//! Displays a formatted error message.
/*! This function uses the virtual URobot::display() function to make the
 message printing robot-specific.

 It formats the output in a standard URBI way by adding an ERROR key
 between brackets at the end.
 */
void
UServer::error(const char* s, ...)
{
  va_list args;
  va_start(args, s);
  vecho_key("ERROR", s, args);
  va_end(args);
}

//! Displays a formatted message.
/*! This function uses the virtual URobot::display() function to make the
 message printing robot-specific.

 It formats the output in a standard URBI way by adding an empty key
 between brackets at the end. If you want to specify a key, use the
 echoKey() function.
 \param s is the formatted string containing the message.
 \sa echoKey()
 */
void
UServer::echo(const char* s, ...)
{
  va_list args;
  va_start(args, s);
  vecho_key("     ", s, args);
  va_end(args);
}

//! Displays a raw message for debug
/*! This function uses the virtual URobot::display() function to make the
 message printing robot-specific.

 \param s is the formatted string containing the message
 \param args Arguments for the format string.
 */
void
UServer::vdebug (const char* s, va_list args)
{
  char buf[MAXSIZE_INTERNALMESSAGE];
  vsnprintf(buf, sizeof buf, s, args);
  effectiveDisplay(buf);
}

void
UServer::debug(const char* s, ...)
{
  va_list args;
  va_start(args, s);
  vdebug (s, args);
  va_end(args);
}

//! Overload this function to specify how your robot is displaying messages.
void
UServer::effectiveDisplay(const char*)
{
}

void
UServer::display(const char* s)
{
  if (debugOutput)
    effectiveDisplay(s);
}

void
UServer::display(const char** b)
{
  for (int i = 0; b[i]; ++i)
    display (b[i]);
}

//! Overload this function to specify how your system will reboot
void
UServer::reboot()
{
}

//! Overload this function to specify how your system will shutdown
void
UServer::shutdown()
{
  scheduler_->killall_jobs ();
}

//! Overload this function to return the running time of the server.
/*! The running time of the server must be in milliseconds.
 */
libport::utime_t
UServer::getTime()
{
  return 0;
}

//! Overload this function to return the remaining power of the robot
/*! The remaining power is expressed as percentage. 0 for empty batteries
 and 1 for full power.
 */
ufloat
UServer::getPower()
{
  return 1;
}

//! Update the server's time using the robot-specific implementation
/*! It is necessary to have an update of the server time to
 increase the performance of successive calls to getTime.
 It allows also to see a whole processing session (like the
 processing of the command tree) as occuring AT the same time,
 from the server's point of view.
 */
void
UServer::updateTime()
{
  lastTime_ = getTime();
}

void
UServer::getCustomHeader (int, char* header, int)
{
  header[0] = 0; // empty string
}

namespace
{
  bool
  file_readable (const std::string& s)
  {
    std::ifstream is (s.c_str(), std::ios::binary);
    bool res = is;
    is.close();
    return res;
  }
}

std::string
UServer::find_file (const libport::path& path)
{
  return search_path.find_file(path) / path.basename();
}

UErrorValue
UServer::loadFile (const std::string& base, UQueue& q, QueueType type)
{
  std::istream *is;
  bool isStdin = (base == std::string("/dev/stdin"));
  if (isStdin)
    is = & std::cin;
  else
  {
    try
    {
      is = new std::ifstream(find_file (base).c_str (), std::ios::binary);
    }
    catch (libport::file_library::Not_found&)
    {
      return UFAIL;
    }
    if (!*is)
      return UFAIL;
  }
  if (type == QUEUE_URBI)
    q.push ((boost::format ("#push 1 \"%1%\"\n") % base).str().c_str());
  while (is->good ())
  {
    char buf[BUFSIZ];
    is->read (buf, sizeof buf);
    q.push(buf, is->gcount());
  }
  if (type == QUEUE_URBI)
    q.push ("#pop\n");
  if (!isStdin)
  {
    reinterpret_cast<std::ifstream*>(is)->close();
    delete is;
  }
  return USUCCESS;
}


//! Add a new connection to the connection list
/*! This function perform also some error testing on the connection
 value and UError return code
 */
void
UServer::addConnection(UConnection& connection)
{
  if (connection.uerror_ != USUCCESS)
    error(::DISPLAY_FORMAT1, (long)this,
	  __PRETTY_FUNCTION__,
	  "UConnection constructor failed");
  else
    connectionList.push_front (&connection);
}

//! Remove a connection from the connection list
/*! This function perform also some error testing on the connection
 value and UError return code
 */
void
UServer::removeConnection(UConnection& connection)
{
  connectionList.remove (&connection);
  echo(::DISPLAY_FORMAT1, (long)this,
       __PRETTY_FUNCTION__,
       "Connection closed", (long) &connection);
  delete &connection;
}


UConnection&
UServer::getGhostConnection ()
{
  return *ghost_;
}


runner::Runner&
UServer::getCurrentRunner () const
{
  return dynamic_cast<runner::Runner&> (scheduler_->current_job());
}


/* Free standing functions. */

namespace
{

  // Use with care, returns a static buffer.
  const char* tab (unsigned n)
  {
    static char buf[1024];
    if (n >= sizeof buf)
      n = sizeof buf - 1;

    for (unsigned i = 0; i < n; ++i)
      buf[i] = ' ';
    buf[n] = 0;
    return buf;
  }

}


void
vdebug (const char* fmt, va_list args)
{
  ::urbiserver->vdebug (fmt, args);
}

void
debug (const char* fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  vdebug (fmt, args);
  va_end (args);
}


void
debug (unsigned t, const char* fmt, ...)
{
  debug ("%s", tab(t));
  va_list args;
  va_start (args, fmt);
  vdebug (fmt, args);
  va_end (args);
}
