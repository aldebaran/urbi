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
// #define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include <cassert>
#include <cstdlib>
#include "libport/cstdio"
#include <cstdarg>

#include <fstream>
#include <sstream>
#include <string>

#include "libport/containers.hh"

#include "ubanner.hh"
#include "uconnection.hh"
#include "ueventcompound.hh"
#include "ueventhandler.hh"
#include "ueventmatch.hh"
#include "ufunction.hh"
#include "ughostconnection.hh"
#include "urbi/uobject.hh"
#include "urbi/usystem.hh"
#include "userver.hh"
#include "utypes.hh"
#include "uvalue.hh"
#include "uvariable.hh"

// Global server reference
UServer *urbiserver= 0;
UString **globalDelete = 0;

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
    isolate_ (false)
{
  ::urbiserver = 0;
  memoryOverflow = securityBuffer_ == 0;
  usedMemory = 0;
  availableMemory = freeMemory;
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
    ghost = new UGhostConnection(this);
    connectionList.push_front(ghost);

    std::ostringstream o;
    o << 'U' << (long)ghost;

    new UVariable(MAINDEVICE, "ghostID", o.str().c_str());
    new UVariable(MAINDEVICE, "name", mainName_.str());
    uservarState = true;
    DEBUG (("done\n"));
  }

  // Plugins (internal components)
  {
    DEBUG (("Loading objecthubs..."));
    for (urbi::UStartlistHub::iterator i = urbi::objecthublist->begin();
	 i != urbi::objecthublist->end();
	 ++i)
      (*i)->init((*i)->name);
    DEBUG (("done\n"));

    DEBUG (("Loading hubs..."));
    for (urbi::UStartlist::iterator i = urbi::objectlist->begin();
	 i != urbi::objectlist->end();
	 ++i)
      (*i)->init((*i)->name);
    DEBUG (("done\n"));
  }

  DEBUG (("Loading URBI.INI..."));
  if (loadFile("URBI.INI", &ghost->recvQueue()) == USUCCESS)
    ghost->newDataAdded = true;
  DEBUG (("done\n"));
}


void
UServer::main (int argc, const char* argv[])
{
  UValue* arglistv = new UValue ();

  UValue* current = 0;
  arglistv->dataType = DATA_LIST;
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
UServer::work()
{
  ECHO("Work in...");
  libport::BlockLock bl(this);
  // CPU Overload test
  updateTime();
  previous3Time = previous2Time;
  previous2Time = previousTime;
  previousTime  = currentTime;
  currentTime   = lastTime();

  // Execute Timers
  for (urbi::UTimerTable::iterator i = urbi::timermap->begin();
       i != urbi::timermap->end();
       ++i)
    if ((*i)->lastTimeCalled - currentTime + (*i)->period < frequency_ / 2)
    {
      (*i)->call();
      (*i)->lastTimeCalled = currentTime;
    }


  beforeWork();
  // Access & Change variable list
  for (std::list<UVariable*>::iterator i = access_and_change_varlist.begin ();
       i != access_and_change_varlist.end ();
       ++i)
    (*i)->get (true);

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
  bool signalMemoryOverflow = false;
  if (memoryOverflow && securityBuffer_)
    {
      // free space to ensure the warning messages will
      // be sent without problem.
      free (securityBuffer_);
      securityBuffer_ = 0;
      signalMemoryOverflow = true;
    }

  // Scan currently opened connections for ongoing work
  for (std::list<UConnection*>::iterator r = connectionList.begin();
       r != connectionList.end();
       ++r)
    if ((*r)->isActive())
    {
      if (!(*r)->isBlocked())
	(*r)->continueSend();

      if (signalMemoryOverflow)
	(*r)->errorSignal(UERROR_MEMORY_OVERFLOW);
      if (signalcpuoverload)
      {
	(*r)->errorSignal(UERROR_CPU_OVERLOAD);
	signalcpuoverload = false;
      }

      (*r)->errorCheck(UERROR_MEMORY_OVERFLOW);
      (*r)->errorCheck(UERROR_MEMORY_WARNING);
      (*r)->errorCheck(UERROR_SEND_BUFFER_FULL);
      (*r)->errorCheck(UERROR_RECEIVE_BUFFER_FULL);
      (*r)->errorCheck(UERROR_RECEIVE_BUFFER_CORRUPTED);
      (*r)->errorCheck(UERROR_CPU_OVERLOAD);

      // Run the connection's command queue:
      if ((*r)->activeCommand)
      {
	(*r)->obstructed = true; // will be changed to 'false'
	//if the whole tree is visited
	(*r)->treeLock.lock();
	(*r)->inwork = true;   // to distinguish this call of
	//execute from the one in receive
	(*r)->execute((*r)->activeCommand);
	(*r)->inwork = false;
	(*r)->treeLock.unlock();
      }

      if ((*r)->newDataAdded)
      {
	// used by loadFile and exec to
	// delay the parsing after the completion
	// of execute().
	(*r)->newDataAdded = false;
	(*r)->received("");
      }
    }

  // Scan currently opened connections for deleting marked
  // commands or killall order
  if (reseting && stage==0)
    stopall = true;

  for (std::list<UConnection*>::iterator r = connectionList.begin();
       r != connectionList.end();
       ++r)
    if ((*r)->isActive() && (*r)->activeCommand)
    {
      if ((*r)->killall || stopall)
      {
	(*r)->killall = false;
	delete (*r)->activeCommand;
	(*r)->activeCommand = 0;
      }
      else if ((*r)->activeCommand->toDelete)
      {
	delete (*r)->activeCommand;
	(*r)->activeCommand = 0;
      }
      else if (somethingToDelete)
	(*r)->activeCommand->deleteMarked();
    }

  somethingToDelete = false;
  stopall = false;

  // Values final assignment and nbAverage reset to 0
  for (std::list<UVariable*>::iterator i = reinitList.begin();
       i != reinitList.end();)
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
	    (*i)->selfSet (&((*i)->target));
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

  // Execute Hub Updaters
  for (urbi::UTimerTable::iterator i = urbi::updatemap->begin();
       i != urbi::updatemap->end();
       ++i)
    if ((*i)->lastTimeCalled - currentTime + (*i)->period < frequency_ / 2)
    {
      (*i)->call();
      (*i)->lastTimeCalled = currentTime;
    }

  // after work
  afterWork();

  updateTime();
  latestTime = lastTime();

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

  // Reseting procedure
  if (reseting)
  {
    ++stage;
    if (stage == 1)
    {
      //delete objects first
      for (HMvariabletab::iterator i = variabletab.begin();
	   i != variabletab.end();
	   ++i)
	if (i->second->value
	    && i->second->value->dataType == DATA_OBJ)
	  varToReset.push_back(i->second);

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

      //delete the rest
      for (HMvariabletab::iterator i = variabletab.begin();
	   i != variabletab.end();
	   ++i)
	if (i->second->uservar)
	  varToReset.push_back(i->second);

      libport::deep_clear (varToReset);

      //variabletab.clear();
      aliastab.clear();
      emittab.clear();
      functiontab.clear();  //This leaks awfuly...
      grouptab.clear();
      objaliastab.clear();
      tagtab.clear();

      for (std::list<UConnection*>::iterator i = connectionList.begin();
	   i != connectionList.end();
	   ++i)
	if ((*i)->isActive())
	  (*i)->send("*** Reset completed.\n", "reset");

      //restart everything
      for (urbi::UStartlist::iterator i = urbi::objectlist->begin();
	   i != urbi::objectlist->end();
	   ++i)
	(*i)->init((*i)->name);

      loadFile("URBI.INI", &ghost->recvQueue());
      char resetsignal[255];
      strcpy(resetsignal, "var __system__.resetsignal;");
      ghost->recvQueue().push((const ubyte*)resetsignal, strlen(resetsignal));
      ghost->newDataAdded = true;
    }
    else
    {
      HMvariabletab::iterator findResetSignal
	= variabletab.find("__system__.resetsignal");
      if (findResetSignal != variabletab.end())
      {
	for (std::list<UConnection*>::iterator i = connectionList.begin();
	     i != connectionList.end();
	     ++i)
	  if ((*i)->isActive() && (*i) != ghost)
	  {
	    (*i)->send("*** Reloading\n", "reset");

	    loadFile("CLIENT.INI", &(*i)->recvQueue());
	    (*i)->newDataAdded = true;
	  }
	reseting = false;
	stage = 0;
      }
    }
  }
  ECHO("Work... done");
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

//! Overload this function to return a specific header for your URBI server
/*! Used to give some information specific to your server in the standardized
 header which is displayed on the server output at start and in the
 connection when a new connection is created.\n
 Typical custom header should be like:

 *** URBI version xx.xx for \<robotname\> robot\\n\n
 *** (c) Copyright \<year\> \<name\>\\n

 The function should return in header the line corresponding to 'line'
 or an empty string (not NULL!) when there is no line any more.
 Each line is supposed to end with a carriage return \\n and each line should
 start with three empty spaces. This complicated method is necessary to allow
 the connection to stamp every line with the standard URBI prefix [time:tag].

 \param line is the requested line number
 \param header the custom header
 \param maxlength the maximum length allowed for the header (the parameter
 has been malloc'ed for that size). Typical size is 1024 octets and
 should be enough for any reasonable header.
 */
void
UServer::getCustomHeader (int, char* header, int)
{
  header[0] = 0; // empty string
}

//! Check if there is enough free memory to run
/*! Every time there is a new, a malloc, a delete or free or a strdup in the
 server, the global variable "usedMemory" is updated. The "memoryCheck"
 function checks that the currently used memory is less than the maximum
 availableMemory declared at the the server initialization.
 If it is more than the maximum, an memoryOverflow is raised and the
 server enter isolation mode.

 \sa isIsolated()
 */
void
UServer::memoryCheck ()
{
  static bool warningSent = false;

  if (usedMemory > availableMemory)
  {
    memoryOverflow = true;
    isolate_ = true;
  }

  // Issue a warning when memory reaches 80% of use (except if you know what
  // you are doing, you better take appropriate measures when this warning
  // reaches your connection...

  if (usedMemory > (int)(0.8 * availableMemory) && !warningSent)
  {
    warningSent = true;

    // Scan currently opened connections
    for (std::list<UConnection*>::iterator i = connectionList.begin();
	 i != connectionList.end();
	 ++i)
      if ((*i)->isActive())
	(*i)->warning(UWARNING_MEMORY);
  }

  // Hysteresis mechanism
  if (usedMemory < (int)(0.7 * availableMemory) && warningSent)
    warningSent = false;
}

//! Evaluate how much memory is available for a malloc
/*! This function tries to evaluate how much memory is available for a malloc,
 using brute force dichotomic allocation. This is the only known way to get
 this information on most systems (like OPENR).
 */
int
UServer::memory()
{
  int memo;
  int memo1;
  int memo2;
  void *buf;

  memo  = 50000000;
  memo1 = 0;
  memo2 = memo;
  while (memo2 > memo1+1)
  {
    memo = (memo1 + memo2)/2;
    buf = malloc(memo);
    if (buf)
    {
      free(buf);
      memo1 = memo;
    }
    else
      memo2 = memo;
  }

  return memo;
}

//! Get a variable in the hash table
UVariable*
UServer::getVariable (const char *device,
		      const char *property)
{
  std::ostringstream o;
  o << device << '.' << property;
  HMvariabletab::iterator hmi = variabletab.find(o.str().c_str());
  if (hmi != variabletab.end())
    return hmi->second;
  else
    return 0;
}



//! Mark all commands with stopTag in all connection for deletion
void
UServer::mark(UString* stopTag)
{
  HMtagtab::iterator it = tagtab.find(stopTag->str());
  if (it == tagtab.end())
    return; //no command with this tag
  TagInfo* ti = &it->second;
  mark(ti);
}

void
UServer::mark(TagInfo* ti)
{
  for (std::list<UCommand*>::iterator i = ti->commands.begin();
      i != ti->commands.end();
      ++i)
    if ((*i)->status != UCommand::UONQUEUE || (*i)->morphed)
      (*i)->toDelete = true;

  for (std::list<TagInfo*>::iterator i = ti->subTags.begin();
       i != ti->subTags.end();
       ++i)
    mark(*i);
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

std::string
UServer::find_file (const char* base)
{
  assert(base);
  //DEBUG(("Looking for file %s\n", base));
  for (path_type::iterator p = path.begin(); p != path.end(); ++p)
  {
    std::string f = *p + "/" + base;
    std::ifstream is (f.c_str(), std::ios::binary);
    if (is)
    {
      is.close ();
      //DEBUG(("File %s found: %s\n", base, f.c_str()));
      return f;
    }
  }
  //DEBUG(("File %s not found in path\n", base));
  return base;
}

#define URBI_BUFSIZ 1024

UErrorValue
UServer::loadFile (const char* base, UCommandQueue* q)
{
  std::string f = find_file (base);
  std::ifstream is (f.c_str(), std::ios::binary);
  if (!is)
    return UFAIL;

  while (is.good ())
  {
    char buf[URBI_BUFSIZ];
    is.read (buf, sizeof buf);
    if (q->push((const ubyte*) buf, is.gcount()) == UFAIL)
      return UFAIL;
  }
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
    newobj = getobj->second->str();
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
    static char buf[100];
    assert(n < sizeof buf);
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
