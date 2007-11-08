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

#include <cassert>
#include <cstdlib>
#include "libport/cstdio"
#include <cstdarg>

#include <fstream>
#include <sstream>
#include <string>
#include <boost/thread.hpp>

#include <boost/foreach.hpp>

#include "libport/containers.hh"
#include "libport/separator.hh"

#include "urbi/uobject.hh"
#include "urbi/usystem.hh"

#include "kernel/userver.hh"
#include "kernel/utypes.hh"
#include "kernel/uvalue.hh"
#include "kernel/uvariable.hh"

#include "ast/ast.hh"
#include "ast/nary.hh"

#include "ubanner.hh"
#include "ucommandqueue.hh"
#include "uqueue.hh"
#include "ueventcompound.hh"
#include "ueventhandler.hh"
#include "ueventmatch.hh"
#include "ufunction.hh"
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
    resetting (false),
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
    isolate_ (false),
    uid(0)
{
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

  if (load_init_file("urbi.u") == USUCCESS)
  {
    ghost_->newDataAdded = true;
    ghost_->recvQueue().push ("#__LINE__1\n");
  }
  load_init_file("URBI.INI");
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

  new UVariable (MAINDEVICE, "args", arglistv);
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
  boost::recursive_mutex::scoped_lock lock(mutex);
  // CPU Overload test
  updateTime ();
  previous3Time = previous2Time;
  previous2Time = previousTime;
  previousTime  = currentTime;
  currentTime   = lastTime ();

  work_exec_timers_ ();

  beforeWork ();

  work_access_and_change_ ();
  work_handle_connections_ ();
  work_handle_stopall_ ();
  work_blend_values_ ();
  work_execute_hub_updater_ ();

  scheduler_->work ();

  afterWork ();

  updateTime ();
  latestTime = lastTime ();

  work_test_cpuoverload_ ();

  work_reset_if_needed_ ();
}

void
UServer::work_exec_timers_ ()
{
  for (urbi::UTimerTable::iterator i = urbi::timermap->begin();
       i != urbi::timermap->end();
       ++i)
    if ((*i)->lastTimeCalled - currentTime + (*i)->period < frequency_ / 2)
    {
      (*i)->call();
      (*i)->lastTimeCalled = currentTime;
    }
}

void
UServer::work_access_and_change_ ()
{
  for (std::list<UVariable*>::iterator i = access_and_change_varlist.begin ();
       i != access_and_change_varlist.end ();
       ++i)
    (*i)->get ();
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
          boost::try_mutex::scoped_lock((*c)->treeMutex);
          c->inwork = true;   // to distinguish this call of
          //execute from the one in receive
          c->execute((*c)->activeCommand);
          c->inwork = false;
        }
      }
#endif

      if (c->newDataAdded)
      {
	// used by loadFile and exec to
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
  if (resetting && stage==0)
    stopall = true;

  BOOST_FOREACH (UConnection* c, connectionList)
    if (c->isActive() && c->has_pending_command ())
    {
      if (c->killall || stopall)
      {
	c->killall = false;
	c->drop_pending_commands ();
      }
    }

  somethingToDelete = false;
  stopall = false;
}

void
UServer::work_blend_values_ ()
{
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
	  {
	    (*i)->selfSet (&((*i)->target));
	    (*i)->setTarget();
	  }
	}

      // set previous for next iteration
      (*i)->previous3 = (*i)->previous2;
      (*i)->previous2 = (*i)->previous;
      (*i)->previous  = (*i)->target;

      if ((*i)->speedmodified)
	(*i)->reloop = true;
      (*i)->speedmodified = false;

      ++i;
    }
}

void
UServer::work_execute_hub_updater_ ()
{
  for (urbi::UTimerTable::iterator i = urbi::updatemap->begin();
       i != urbi::updatemap->end();
       ++i)
    if ((*i)->lastTimeCalled - currentTime + (*i)->period < frequency_ / 2)
    {
      (*i)->call();
      (*i)->lastTimeCalled = currentTime;
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
  if (!resetting)
    return;
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

    //delete hubs
    for (urbi::UStartlistHub::iterator i = urbi::objecthublist->begin();
	 i != urbi::objecthublist->end();
	 ++i)
      delete (*i)->getUObjectHub ();

    //delete the rest
    for (HMvariabletab::iterator i = variabletab.begin();
	 i != variabletab.end();
	 ++i)
      if (i->second->uservar)
	varToReset.push_back(i->second);

    libport::deep_clear (varToReset);

    aliastab.clear();
    emittab.clear();
    functiontab.clear();  //This leaks awfully...
    grouptab.clear();
    objaliastab.clear();

    // do not clear tagtab, everything is refcounted and will be cleared
    // when commands will be
    //tagtab.clear();

    for (std::list<UConnection*>::iterator i = connectionList.begin();
         i != connectionList.end();
         ++i)
      if ((*i)->isActive())
        (**i) << UConnection::send("*** Reset completed. Now, restarting...\n", "reset");

    //restart uobjects
    for (urbi::UStartlist::iterator i = urbi::objectlist->begin();
	 i != urbi::objectlist->end();
	 ++i)
      (*i)->init((*i)->name);

    //reload URBI.INI
    loadFile("URBI.INI", &ghost_->recvQueue());
    ghost_->recvQueue().push ("#__LINE__1\n");
    char resetsignal[255];
    strcpy(resetsignal, "var __system__.resetsignal;");
    ghost_->recvQueue().push((const ubyte*)resetsignal, strlen(resetsignal));
    ghost_->newDataAdded = true;
  }
  else if (libport::mhas(variabletab, "__system__.resetsignal"))
  {
    //reload CLIENT.INI
    for (std::list<UConnection*>::iterator i = connectionList.begin();
         i != connectionList.end();
         ++i)
      if ((*i)->isActive() && (*i) != ghost_)
      {
        (**i) << UConnection::send("*** Restart completed.\n", "reset");
        loadFile("CLIENT.INI", &(*i)->recvQueue());
        (*i)->recvQueue().push ("#__LINE__1\n");
        (*i)->newDataAdded = true;
        (**i) << UConnection::send("*** Ready.\n", "reset");
      }
    resetting = false;
    stage = 0;
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

//! Get a variable in the hash table
UVariable*
UServer::getVariable (const std::string& device,
		      const std::string& property)
{
  std::string n = device + '.' + property;
  return libport::find0 (variabletab, n.c_str());
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
UServer::mark(TagInfo* /*ti*/)
{
#if 0
  for (std::list<UCommand*>::iterator i = ti->commands.begin();
      i != ti->commands.end();
      ++i)
    if ((*i)->status != UCommand::UONQUEUE || (*i)->morphed)
      (*i)->toDelete = true;

  for (std::list<TagInfo*>::iterator i = ti->subTags.begin();
       i != ti->subTags.end();
       ++i)
    mark(*i);
#endif
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
UServer::find_file (const std::string& base)
{
  ECHO (base << " in " << libport::separate(path, ':'));
  for (path_type::iterator p = path.begin(); p != path.end(); ++p)
  {
    std::string f = *p + '/' + base;
    if (file_readable(f))
    {
      ECHO("found: " << f);
      return f;
    }
  }
  if (!file_readable(base))
  {
    ECHO("not found: " << base);
    error ((std::string ("cannot find file: ") + base).c_str ());
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

  while (is.good ())
  {
    char buf[BUFSIZ];
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

//! Adds an alias in the server base
/*! This function is mostly useful for alias creation at start in the
 initialize() function for example.
 return 1 in case of success,  0 if circular alias is detected
 */
int
UServer::addAlias(const std::string& id, const std::string& variablename)
{
  if (id == variablename)
    return 0;

  const char* newobj = variablename.c_str ();
  HMaliastab::iterator getobj = ::urbiserver->aliastab.find(newobj);

  while (getobj != ::urbiserver->aliastab.end())
  {
    newobj = getobj->second->c_str();
    if (id == newobj)
      return 0;

    getobj = ::urbiserver->aliastab.find(newobj);
  }

  if (aliastab.find(id.c_str ()) != aliastab.end())
  {
    UString* alias = aliastab[id.c_str ()];
    *alias = variablename.c_str ();
  }
  else
  {
    // XXX we'll leak id_copy forever :|
    char* id_copy = strdup (id.c_str ());
    assert (id_copy);
    aliastab[id_copy] = new UString (variablename.c_str ());
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
