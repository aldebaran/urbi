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

#include "libport/containers.hh"

#include "urbi/uobject.hh"
#include "urbi/usystem.hh"

#include "kernel/userver.hh"
#include "kernel/utypes.hh"
#include "kernel/uvalue.hh"
#include "kernel/uvariable.hh"

#include "config.h"
#include "ubanner.hh"
#include "ucommand.hh"
#include "ucommandqueue.hh"
#include "uqueue.hh"
#include "ueventcompound.hh"
#include "ueventhandler.hh"
#include "ueventmatch.hh"
#include "ufunction.hh"
#include "ughostconnection.hh"

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
		 int freeMemory,
		 const char* mainName)
  : reseting (false),
    stage (0),
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
    securityBuffer_ (malloc(SECURITY_MEMORY_SIZE)),
    isolate_ (false),
    uid(0)
{
  ::urbiserver = 0;
  memoryOverflow = securityBuffer_ == 0;
  usedMemory = 0;
  availableMemory = freeMemory;
  // FIXME: What the heck???  We don't even check if it fits!!!
  availableMemory -=  3000000; // Need 3.1Mo at min to run safely.
  // You might hack this for a small
  // size server, but anything with
  // less than 3Mb of memory is not a
  // good candidate to run URBI anyway...
  if (availableMemory < 100000)
    memoryOverflow = true;

  ADDOBJ(UServer);
  ::urbiserver = this;

  // Create system events
  kernel::eh_system_alwaystrue = new UEventHandler
    (new UString ("system.alwaysTrue"), 0);
  kernel::eh_system_alwaysfalse = new UEventHandler
    (new UString ("system.alwaysFalse"), 0);

  std::list<UValue*> args;
  kernel::system_alwaystrue = new UEvent (kernel::eh_system_alwaystrue, args);
  kernel::eh_system_alwaystrue->addEvent (kernel::system_alwaystrue);

  kernel::eventmatch_true  = new UEventMatch (kernel::eh_system_alwaystrue);
  kernel::eventmatch_false = new UEventMatch (kernel::eh_system_alwaysfalse);
  kernel::remoteFunction   = new UFunction("<remote Function>", 0, 0);

  // initialize system message channels
  std::list<urbi::USystem*> empty_list;
  systemObjects.reserve (1); // one system message type known so far.
  systemObjects.push_back (empty_list);
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

    new UVariable(MAINDEVICE, "ghostID", o.str().c_str());
    new UVariable(MAINDEVICE, "name", mainName_.c_str());
    uservarState = true;
    DEBUG (("done\n"));
  }

  // Cached taginfos.
  TagInfo::initializeTagInfos();

  // Plugins (internal components)
  {
    DEBUG (("Loading objecthubs..."));
    BOOST_FOREACH (urbi::baseURBIStarterHub* i, *urbi::objecthublist)
      i->init(i->name);
    DEBUG (("done\n"));

    DEBUG (("Loading hubs..."));
    BOOST_FOREACH (urbi::baseURBIStarter* i, *urbi::objectlist)
      i->init(i->name);
    DEBUG (("done\n"));
  }

  DEBUG (("Loading URBI.INI..."));
  if (loadFile("URBI.INI", &ghost_->recvQueue()) == USUCCESS)
  {
    ghost_->newDataAdded = true;
    ghost_->recvQueue().push ("#line 1 \"\"\n");
  }
  DEBUG (("done\n"));
}


void
UServer::main (int argc, const char* argv[])
{
  UValue* arglistv = new UValue ();
  arglistv->dataType = DATA_LIST;

  UValue* current = 0;
  for (int i = 0; i < argc; ++i)
  {
    UValue* v = new UValue (argv[i]);
    if (i == 0)
      arglistv->liststart = v;
    else
      current->next = v;
    current = v;
  }

  new UVariable(MAINDEVICE, "args", arglistv);
}

//! Function called before work
/*! Redefine this virtual function if you need to do pre-processing before
 the work function starts.
 */
void
UServer::beforeWork()
{
}

//! Function called after work
/*! Redefine this virtual function if you need to do post-processing before
 the work function ends.
 */
void
UServer::afterWork()
{
}


void
UServer::work_exec_timers_ ()
{
  BOOST_FOREACH (urbi::UTimerCallback* i, *urbi::timermap)
    if (i->lastTimeCalled - currentTime + i->period < frequency_ / 2)
    {
      i->call();
      i->lastTimeCalled = currentTime;
    }
}

void
UServer::work_access_and_change_ ()
{
  BOOST_FOREACH (UVariable* i, access_and_change_varlist)
    i->get ();
}

bool
UServer::work_memory_check_ ()
{
  // memory test
  memoryCheck(); // Check for memory availability
  // recover the security memory space, with a margin (size x 2)
  // if the margin can be malloced, then freed, then remalloced with
  // the correct size, then the memoryOverflow error is removed.
  if (securityBuffer_ == 0 &&
      usedMemory < availableMemory - 2 * SECURITY_MEMORY_SIZE)
  {
    securityBuffer_ = malloc(2 * SECURITY_MEMORY_SIZE);
    if (securityBuffer_ != 0)
    {
      free(securityBuffer_);
      securityBuffer_ = malloc(SECURITY_MEMORY_SIZE);
      if (securityBuffer_)
      {
	memoryOverflow = false;
	deIsolate();
      }
    }
  }

  if (memoryOverflow && securityBuffer_)
  {
    // Free space to ensure the warning messages will be sent without
    // problem.
    free (securityBuffer_);
    securityBuffer_ = 0;
    return true;
  }

  return false;
}

void
UServer::work_handle_connections_ (bool overflow)
{
  // Scan currently opened connections for ongoing work
  BOOST_FOREACH (UConnection* r, connectionList)
    if (r->isActive())
    {
      if (!r->isBlocked())
	*r << UConnection::continueSend;

      if (overflow)
	*r << UConnection::errorSignal(UERROR_MEMORY_OVERFLOW);
      if (signalcpuoverload)
      {
	*r << UConnection::errorSignal(UERROR_CPU_OVERLOAD);
	signalcpuoverload = false;
      }

      *r << UConnection::errorCheck(UERROR_MEMORY_OVERFLOW);
      *r << UConnection::errorCheck(UERROR_MEMORY_WARNING);
      *r << UConnection::errorCheck(UERROR_SEND_BUFFER_FULL);
      *r << UConnection::errorCheck(UERROR_RECEIVE_BUFFER_FULL);
      *r << UConnection::errorCheck(UERROR_RECEIVE_BUFFER_CORRUPTED);
      *r << UConnection::errorCheck(UERROR_CPU_OVERLOAD);

      // Run the connection's command queue:
      if (r->activeCommand)
      {
	r->obstructed = true; // will be changed to 'false'
	{
	  //if the whole tree is visited
# if ! defined LIBPORT_URBI_ENV_AIBO
	  boost::try_mutex::scoped_lock(r->treeMutex);
# endif
	  r->inwork = true;   // to distinguish this call of
	  //execute from the one in receive
	  r->execute(r->activeCommand);
	  r->inwork = false;
	}
      }

      if (r->newDataAdded)
      {
	// used by loadFile and eval to
	// delay the parsing after the completion
	// of execute().
	r->newDataAdded = false;
	*r << UConnection::received("");
      }
    }
}

void
UServer::work_handle_stopall_ ()
{
  // Scan currently opened connections for deleting marked
  // commands or killall order
  if (reseting && stage==0)
    stopall = true;

  BOOST_FOREACH (UConnection* r, connectionList)
    if (r->isActive() && r->activeCommand)
    {
      if (r->killall || stopall)
      {
	r->killall = false;
	delete r->activeCommand;
	r->activeCommand = 0;
      }
      else if (r->activeCommand->toDelete)
      {
	delete r->activeCommand;
	r->activeCommand = 0;
      }
      else if (somethingToDelete)
	r->activeCommand->deleteMarked();
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
UServer::work_blend_values_ ()
{
  // Values final assignment and nbAverage reset to 0
  for (std::list<UVariable*>::iterator i = reinitList.begin();
       i != reinitList.end();)
  {
    if ((*i)->activity == 2)
    {
      (*i)->activity = 0;
      // set previous for stationnary values
      (*i)->previous3 = (*i)->previous;
      (*i)->previous2 = (*i)->previous;

      i = reinitList.erase(i);
    }
    else
    {
      (*i)->activity = 2;
      (*i)->nbAverage = 0;
      (*i)->reloop = false;

      if ((*i)->blendType == urbi::UMIX || (*i)->blendType == urbi::UADD)
	if ((*i)->value->dataType == DATA_NUM)
	{
	  if ((*i)->autoUpdate)
	    (*i)->selfSet (&((*i)->value->val));
	  else
	  {
	    (*i)->selfSet (&((*i)->target));
	    (*i)->setTarget();
	  }
	}

      // set previous for next iation
      (*i)->previous3 = (*i)->previous2;
      (*i)->previous2 = (*i)->previous;
      (*i)->previous  = (*i)->target;

      if ((*i)->speedmodified)
	(*i)->reloop = true;
      (*i)->speedmodified = false;

      ++i;
    }
  }
}

void
UServer::work_execute_hub_updater_ ()
{
  // Execute Hub Updaters
  BOOST_FOREACH (urbi::UTimerCallback* i, *urbi::updatemap)
    if (i->lastTimeCalled - currentTime + i->period < frequency_ / 2)
    {
      i->call();
      i->lastTimeCalled = currentTime;
    }
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

void
UServer::work_reset_if_needed_ ()
{
  // Reseting procedure
  if (reseting)
  {
    ++stage;
    if (stage == 1)
    {
      //delete objects first
      BOOST_FOREACH (HMvariabletab::value_type i, variabletab)
	if (i.second->value
	    && i.second->value->dataType == DATA_OBJ)
	  varToReset.push_back(i.second);

      while (!varToReset.empty())
	for (std::list<UVariable*>::iterator it = varToReset.begin();
	     it != varToReset.end();)
	  if ((*it)->isDeletable())
	  {
	    delete *it;
	    it = varToReset.erase(it);
	  }
	  else
	    ++it;

      //delete hubs
      BOOST_FOREACH (urbi::baseURBIStarterHub* i, *urbi::objecthublist)
	delete i->getUObjectHub ();


      //delete the rest
      BOOST_FOREACH (HMvariabletab::value_type i, variabletab)
	if (i.second->uservar)
	  varToReset.push_back(i.second);

      libport::deep_clear (varToReset);

      aliastab.clear();
      emittab.clear();
      functiontab.clear();  //This leaks awfuly...
      grouptab.clear();
      objaliastab.clear();

      // do not clear tagtab, everything is refcounted and will be cleared
      // when commands will be
      //tagtab.clear();

      BOOST_FOREACH (UConnection* i, connectionList)
	if (i->isActive())
	  *i << UConnection::send("*** Reset completed. Now, restarting...\n",
				  "reset");

      //restart hubs
      BOOST_FOREACH (urbi::baseURBIStarterHub* i, *urbi::objecthublist)
	i->init(i->name);

      //restart uobjects
      BOOST_FOREACH (urbi::baseURBIStarter* i, *urbi::objectlist)
	i->init(i->name);

      //reload URBI.INI
      loadFile("URBI.INI", &ghost_->recvQueue());
      ghost_->recvQueue().push ("#line 1 \"\"\n");
      char resetsignal[255];
      strcpy(resetsignal, "var __system__.resetsignal;");
      ghost_->recvQueue().push((const ubyte*)resetsignal, strlen(resetsignal));
      ghost_->newDataAdded = true;
    }
    else if (libport::mhas(variabletab, "__system__.resetsignal"))
    {
      //reload CLIENT.INI
      BOOST_FOREACH (UConnection* i, connectionList)
	if (i->isActive() && i != ghost_)
	{
	  *i << UConnection::send("*** Restart completed.\n", "reset");
	  loadFile("CLIENT.INI", &i->recvQueue());
	  i->recvQueue().push ("#line 1 \"\"\n");
	  i->newDataAdded = true;
	  *i << UConnection::send("*** Ready.\n", "reset");
	}
      reseting = false;
      stage = 0;
    }
  }
}

void
UServer::work()
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

  work_exec_timers_ ();

  beforeWork ();

  work_access_and_change_ ();
  bool overflow = work_memory_check_ ();
  work_handle_connections_ (overflow);
  work_handle_stopall_ ();
  work_blend_values_ ();
  work_execute_hub_updater_ ();

  afterWork ();

  updateTime ();
  latestTime = lastTime ();

  work_test_cpuoverload_ ();

  work_reset_if_needed_ ();
}

//! UServer destructor.
UServer::~UServer()
{
  FREEOBJ(UServer);
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

void
UServer::memoryCheck ()
{
  if (usedMemory > availableMemory)
  {
    memoryOverflow = true;
    isolate_ = true;
  }

  // Issue a warning when memory reaches 80% of use (except if you know what
  // you are doing, you better take appropriate measures when this warning
  // reaches your connection...
  static bool warningSent = false;
  if (usedMemory > (int)(0.8 * availableMemory) && !warningSent)
  {
    warningSent = true;

    // Scan currently opened connections
    BOOST_FOREACH (UConnection* i, connectionList)
      if (i->isActive())
	(*i) << UWARNING_MEMORY;
  }

  // Hysteresis mechanism
  if (usedMemory < (int)(0.7 * availableMemory) && warningSent)
    warningSent = false;
}

size_t
UServer::memory() const
{
  size_t low = 0;
  size_t high = 50000000;
  size_t mid = 0;
  while (low + 1 < high)
  {
    mid = (low + high)/2;
    if (void *buf = malloc(mid))
    {
      free(buf);
      low = mid;
    }
    else
      high = mid;
  }

  return mid;
}

//! Get a variable in the hash table
UVariable*
UServer::getVariable (const char *device,
		      const char *property)
{
  std::string n = std::string (device) + "." + property;
  return libport::find0(variabletab, n.c_str());
}



//! Mark all commands with stopTag in all connection for deletion
void
UServer::mark(UString* stopTag)
{
  HMtagtab::iterator it = tagtab.find(stopTag->c_str());
  if (it == tagtab.end())
    return; //no command with this tag
  TagInfo* ti = &it->second;
  mark(ti);
}

void
UServer::mark(TagInfo* ti)
{
  BOOST_FOREACH(UCommand* i, ti->commands)
    if (i->status != UCommand::UONQUEUE || i->morphed)
      i->toDelete = true;

  BOOST_FOREACH (TagInfo* i, ti->subTags)
    mark(i);
}


void
UServer::freeze(const std::string &tag)
{
  HMtagtab::iterator it = tagtab.find(tag);
  if (it != tagtab.end())
    it->second.frozen = true;
  else
  {
    TagInfo t;
    t.name = tag;
    t.frozen = true;
    t.blocked = false;
    t.insert(tagtab);
  }
}

void
UServer::unfreeze(const std::string &tag)
{
  HMtagtab::iterator it = tagtab.find(tag);
  if (it != tagtab.end())
    it->second.frozen = false;
}

void
UServer::block(const std::string &tag)
{
  HMtagtab::iterator it = tagtab.find(tag);
  if (it != tagtab.end())
    it->second.blocked = true;
  else
  {
    TagInfo t;
    t.name = tag;
    t.frozen = false;
    t.blocked = true;
    t.insert(tagtab);
  }
}

void
UServer::unblock(const std::string &tag)
{
  HMtagtab::iterator it = tagtab.find(tag);
  if (it != tagtab.end())
    it->second.blocked = false;
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
UServer::find_file (const char* base)
{
  assert(base);
  //DEBUG(("Looking for file %s\n", base));
  for (path_type::iterator p = path.begin(); p != path.end(); ++p)
  {
    std::string f = *p + "/" + base;
    ECHO("find_file(" << base << ") testing " << f);
    if (file_readable(f))
    {
      ECHO("find_file(" << base << ") = " << f);
      return f;
    }
  }
  if (!file_readable(base))
    error ("cannot find file: %s", base);
  return base;
}

#define URBI_BUFSIZ 1024

UErrorValue
UServer::loadFile (const char* base, UCommandQueue* q, QueueType type)
{
  std::string f = find_file (base);
  std::ifstream is (f.c_str(), std::ios::binary);
  if (!is)
    return UFAIL;

  if (type == QUEUE_URBI)
    q->push ((boost::format ("#push 1 \"%1%\"\n") % base).str().c_str());
  while (is.good ())
  {
    char buf[URBI_BUFSIZ];
    is.read (buf, sizeof buf);
    if (q->push((const ubyte*) buf, is.gcount()) == UFAIL)
      return UFAIL;
  }
  if (type == QUEUE_URBI)
    q->push ("#pop\n");
  is.close();

  return USUCCESS;
}


//! Add a new connection to the connection list
/*! This function perform also some error testing on the connection
 value and UError return code
 */
void
UServer::addConnection(UConnection *connection)
{
  if (!connection || connection->uerror_ != USUCCESS)
    error(::DISPLAY_FORMAT1, (long)this,
	  "UServer::addConnection",
	  "UConnection constructor failed");
  else
    connectionList.push_front(connection);
}

//! Remove a connection from the connection list
/*! This function perform also some error testing on the connection
 value and UError return code
 */
void
UServer::removeConnection(UConnection *connection)
{
  connectionList.remove(connection);
  echo(::DISPLAY_FORMAT1, (long)this,
       "UServer::removeConnection",
       "Connection closed", (long)connection);
  delete connection;
}


UConnection&
UServer::getGhostConnection ()
{
  return *ghost_;
}

//! Adds an alias in the server base
/*! This function is mostly useful for alias creation at start in the
 initialize() function for example.
 return 1 in case of success,  0 if circular alias is detected
 */
int
UServer::addAlias(const char* id, const char* variablename)
{
  if (STREQ(id, variablename))
    return 0;

  const char* newobj = variablename;
  HMaliastab::iterator getobj = ::urbiserver->aliastab.find(newobj);

  while (getobj != ::urbiserver->aliastab.end())
  {
    newobj = getobj->second->c_str();
    if (STREQ(newobj, id))
      return 0;

    getobj = ::urbiserver->aliastab.find(newobj);
  }

  if (aliastab.find(id) != aliastab.end())
  {
    UString* alias = aliastab[id];
    *alias = variablename;
  }
  else
  {
    // XXX we'll leak id_copy forever :|
    char* id_copy = strdup (id);
    assert (id_copy);
    aliastab[id_copy] = new UString(variablename);
  }
  return 1;
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
