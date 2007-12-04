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
#include "libport/compiler.hh"

#include "libport/config.h"
#include <cassert>
#include <cstdlib>
#include "libport/cstdio"
#include <cstdarg>

#include <fstream>
#include <sstream>
#include <string>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#if ! defined LIBPORT_URBI_ENV_AIBO
# include <boost/thread.hpp>
#endif

#include <boost/foreach.hpp>

#include "libport/containers.hh"
#include "libport/separator.hh"

#include "urbi/usystem.hh"

#include "kernel/userver.hh"
#include "kernel/utypes.hh"

#include "ast/ast.hh"
#include "ast/nary.hh"
#include "config.h"
#include "ubanner.hh"
#include "ucommandqueue.hh"
#include "uqueue.hh"
#include "ughostconnection.hh"

#include "runner/scheduler.hh"

// Global server reference
UServer *urbiserver = 0;

const char* EXTERNAL_MESSAGE_TAG   = "__ExternalMessage__";

// Formatting for the echo and error outputs.
const char* DISPLAY_FORMAT   = "[%ld] %-35s %s";
const char* DISPLAY_FORMAT1  = "[%ld] %-35s %s : %ld";
const char* DISPLAY_FORMAT2  = "[%d] %-35s %s : %d/%d";

const char* UNKNOWN_TAG = "notag";
const char* MAINDEVICE  = "system";

// Memory counter system
int availableMemory;
int usedMemory;


UServer::UServer(ufloat frequency,
		 const char* mainName)
  : scheduler_ (new runner::Scheduler),
    debugOutput (false),
    mainName_ (mainName),
    somethingToDelete (false),
    uservarState (false),
    cpuoverload (false),
    signalcpuoverload (false),
    cpucount (0),
    cputhreshold (1.2),
    defcheck (false),
    stopall (false),
    systemcommands (true),
    frequency_(frequency),
    isolate_ (false),
    uid(0)
{
  ::urbiserver = this;
}

UErrorValue
UServer::load_init_file(const char* fn)
{
  DEBUG (("Loading %s...", fn));
  UErrorValue res = loadFile(fn, &ghost_->recvQueue());
  if (res == USUCCESS)
  {
    DEBUG (("done\n"));
    ghost_->newDataAdded = true;
  }
  else
    DEBUG (("not found\n"));
  return res;
}

void
UServer::initialize()
{
  updateTime();
  currentTime = latestTime = lastTime();
  previousTime = currentTime - 0.000001; // avoid division by zero at start
  previous2Time = previousTime - 0.000001; // avoid division by zero at start
  previous3Time = previous2Time - 0.000001; // avoid division by zero at start

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
    } while (customHeader[0]!=0);

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

  if (load_init_file("urbi.u") == USUCCESS)
  {
    ghost_->newDataAdded = true;
    ghost_->recvQueue().push ("#line 1\n");
  }
  load_init_file("URBI.INI");
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

void
UServer::work ()
{
# if ! defined LIBPORT_URBI_ENV_AIBO
  boost::recursive_mutex::scoped_lock lock(mutex);
# endif
  // CPU Overload test
  updateTime ();
  previous3Time = previous2Time;
  previous2Time = previousTime;
  previousTime  = currentTime;
  currentTime   = lastTime ();

  beforeWork ();

  work_handle_connections_ ();

  scheduler_->work ();

  afterWork ();

  updateTime ();
  latestTime = lastTime ();

  work_test_cpuoverload_ ();
}

void
UServer::work_handle_connections_ ()
{
  // Scan currently opened connections for ongoing work
  BOOST_FOREACH (UConnection* c, connectionList)
    if (c->isActive())
    {
      if (!c->isBlocked())
	(*c) << UConnection::continueSend;

      if (signalcpuoverload)
      {
	(*c) << UConnection::errorSignal(UERROR_CPU_OVERLOAD);
	signalcpuoverload = false;
      }

      (*c) << UConnection::errorCheck(UERROR_MEMORY_OVERFLOW);
      (*c) << UConnection::errorCheck(UERROR_MEMORY_WARNING);
      (*c) << UConnection::errorCheck(UERROR_SEND_BUFFER_FULL);
      (*c) << UConnection::errorCheck(UERROR_RECEIVE_BUFFER_FULL);
      (*c) << UConnection::errorCheck(UERROR_RECEIVE_BUFFER_CORRUPTED);
      (*c) << UConnection::errorCheck(UERROR_CPU_OVERLOAD);

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
	  c->inwork = true;   // to distinguish this call of
	  //execute from the one in receive
	  c->execute((*c)->activeCommand);
	  c->inwork = false;
	}
      }
#endif

      if (c->newDataAdded)
      {
	// used by loadFile and eval to
	// delay the parsing after the completion
	// of execute().
	c->newDataAdded = false;
	(*c) << UConnection::received("");
      }
    }
}

void
UServer::work_handle_stopall_ ()
{
  BOOST_FOREACH (UConnection* c, connectionList)
    if (c->isActive() && c->has_pending_command ())
    {
      if (c->killall || stopall)
      {
	c->killall = false;
	c->drop_pending_commands ();
      }
    }

  // Delete all connections with closing=true
  for (std::list<UConnection *>::iterator i = connectionList.begin();
       i!= connectionList.end(); )
  {
    if ((*i)->closing)
    {
      delete *i;
      i = connectionList.erase(i);
    }
    else
      i++;
  }

  somethingToDelete = false;
  stopall = false;
}

void
UServer::work_test_cpuoverload_ ()
{
  cpuload = (latestTime - currentTime)/getFrequency();

  if (!cpuoverload)
    if (cpuload > cputhreshold)
    {
      ++cpucount;
      if (cpucount > 10)
      {
	cpucount = 0;
	cpuoverload = true;
	signalcpuoverload = true;
      }
    }
    else if (cpucount > 0)
      --cpucount;

  if (cpuoverload && cpuload < 1)
  {
    cpuoverload = false;
    cpucount = 0;
  }
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

//! Isolate the server from incoming commands.
void
UServer::isolate()
{
  isolate_ = true;
}

//! Remove the server isolatation from incoming commands.
void
UServer::deIsolate()
{
  isolate_ = false;
}

//! Returns the status of isolation of the server
/*! Usual situation is that the server is not isolated. Isolation comes when
 a critical error happens, like a memory overflow, and the server should
 stop receiving incoming commands. It is the job of the programmer in his/her
 own UServer sub class (robot specific) to test this function before
 requesting more data through the client connection.
 */
bool
UServer::isIsolated()
{
  return isolate_;
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
ufloat
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
UServer::find_file (const std::string& base)
{
  ECHO (base << " in " << libport::separate(path, ':'));
  for (path_type::iterator p = path.begin(); p != path.end(); ++p)
  {
    std::string f = *p + "/" + base;
    ECHO("find_file(" << base << ") testing " << f);
    if (file_readable(f))
    {
      ECHO("found: " << f);
      return f;
    }
  }
  if (!file_readable(base))
  {
    ECHO("not found: " << base);
    error ("cannot find file: %s", base.c_str());
  }
  return base;
}

UErrorValue
UServer::loadFile (const std::string& base, UCommandQueue* q)
{
  std::string f = find_file (base);
  std::ifstream is (f.c_str(), std::ios::binary);
  if (!is)
    return UFAIL;

  q->push ((boost::format ("#push 1 \"%1%\"\n") % base).str().c_str());
  while (is.good ())
  {
    char buf[BUFSIZ];
    is.read (buf, sizeof buf);
    if (q->push((const ubyte*) buf, is.gcount()) == UFAIL)
      return UFAIL;
  }
  q->push ("#pop\n");
  is.close();

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
