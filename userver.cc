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

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <string>

#include "ubanner.hh"
#include "userver.h"
#include "uconnection.h"
#include "utypes.h"
#include "ughostconnection.h"
#include "uobject.h"

#ifdef _MSC_VER
# define snprintf _snprintf
# define vsnprintf _vsnprintf
#endif
// Global server reference
UServer    *urbiserver= 0;
UString    **globalDelete = 0;

const char* EXTERNAL_MESSAGE_TAG   = "__ExternalMessage__";
int URBI_unicID = 10000; ///< unique identifier to create new references

// Formatting for the echo and error outputs.

const char* DISPLAY_FORMAT   = "[%ld] %-35s %s";
const char* DISPLAY_FORMAT1  = "[%ld] %-35s %s : %ld";
const char* DISPLAY_FORMAT2  = "[%d] %-35s %s : %d/%d";

const char* UNKNOWN_TAG = "notag";
const char* MAINDEVICE  = "system";

// Memory counter system

int availableMemory;
int usedMemory;


//! UServer constructor.
/*! UServer constructor

    Unlike UConstructor, it is not required that you handle the memory
    management task when you create the robot-specific sub class. The
    difference in memory between your class and the UServer class is
    considered as neglectible and included in the security margin. If you
    don't understand this point, ignore it.

    \param frequency gives the value in msec of the server update,
    which are the calls to the "work" function. These calls must be done at
    a fixed, precise, real-time frequency to let the server computer motor
    trajectories between two "work" calls.

    \param freeMemory indicates the biggest malloc possible on the system
	   when the server has just started. It is used to determine a high
	   limit of memory allocation, thus avoiding later to run out of memory
	   during a new or malloc.
*/
UServer::UServer(ufloat frequency,
		 int freeMemory,
		 const char* mainName)
{
  uservarState = false;

  ::urbiserver = 0;
  frequency_      = frequency;
  securityBuffer_ = malloc( SECURITY_MEMORY_SIZE );
  this->mainName = new UString(mainName);

  memoryOverflow = securityBuffer_ == 0;

  isolate_        = false;
  debugOutput     = false;

  availableMemory = freeMemory;

  availableMemory -=  3000000; // Need 3.1Mo at min to run safely.
			       // You might hack this for a small
			       // size server, but anything with
			       // less than 3Mb of memory is not a
			       // good candidate to run URBI anyway...
  if (availableMemory < 100000)
    memoryOverflow = true;

  somethingToDelete = false;

  cpuoverload = false;
  cpucount = 0;
  signalcpuoverload = false;
  cputhreshold = 1.2;
  defcheck = false;
  stopall = false;
  systemcommands = true;

  parser.commandTree = 0;
  // init the memory manager.
  usedMemory = 0 ;
  reseting = false;
  stage = 0;

  ADDOBJ(UServer);
  ::urbiserver = this;
}

//! Initialization of the server. Displays the header message & init stuff
/*! This function must be called once the server is operational and
    able to print messages. It is a requirement for URBI compliance to print
    the header at start, so this function *must* be called. Beside, it also
    do initalization work for the devices and system variables.
*/
void
UServer::initialization()
{
  updateTime();
  currentTime = latestTime = lastTime();
  previousTime = currentTime - 0.000001; // avoid division by zero at start
  previous2Time = previousTime - 0.000001; // avoid division by zero at start
  previous3Time = previous2Time - 0.000001; // avoid division by zero at start

  debugOutput     = true;
  display(::HEADER_BEFORE_CUSTOM);

  int i = 0;
  char customHeader[1024];

  do {
    getCustomHeader(i,(char*)customHeader,1024);
    if (customHeader[0])
      display((const char*) customHeader);
    i++;
  } while (customHeader[0]!=0);

  display(::HEADER_AFTER_CUSTOM);
  display("Ready.\n");

  debugOutput     = false;


  //The order is important: ghost connection, plugins, urbi.ini

  // Ghost connection
  ghost  = new UGhostConnection(this);
  connectionList.push_front(ghost);

  char tmpbuffer_ghostTag[50];
  sprintf(tmpbuffer_ghostTag,"U%ld",(long)ghost);

  new UVariable(MAINDEVICE,"ghostID", tmpbuffer_ghostTag);
  new UVariable(MAINDEVICE,"name", mainName->str());
  uservarState = true;


   // Plugins (internal components)

  for (urbi::UStartlistHub::iterator retr = urbi::objecthublist->begin();
      retr != urbi::objecthublist->end();
      retr++)
    (*retr)->init((*retr)->name);

  for (urbi::UStartlist::iterator retr = urbi::objectlist->begin();
      retr != urbi::objectlist->end();
      retr++)
    (*retr)->init((*retr)->name);

  if (loadFile("URBI.INI",ghost->recvQueue()) == USUCCESS)
    ghost->newDataAdded = true;
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


//! Main processing loop of the server
/*! This function must be called every "frequency_" msec to ensure the proper
    functionning of the server. It will call the command execution, the
    connection message sending when they are delayed, etc...

    "frequency_" is a parameter of the server, given in the constructor.
*/
void
UServer::work()
{
  // CPU Overload test
  updateTime();
  previous3Time = previous2Time;
  previous2Time = previousTime;
  previousTime  = currentTime;
  currentTime   = lastTime();

  // Execute Timers
  for (urbi::UTimerTable::iterator ittt = urbi::timermap.begin();
       ittt != urbi::timermap.end();
       ittt++)
    if ((*ittt)->lastTimeCalled - currentTime + (*ittt)->period <
	frequency_ / 2) {

      (*ittt)->call();
      (*ittt)->lastTimeCalled = currentTime;
    }


  beforeWork();

  // memory test
  memoryCheck(); // Check for memory availability

  // recover the security memory space, with a margin (size x 2)
  // if the margin can be malloced, then freed, then remalloced with
  // the correct size, then the memoryOverflow error is removed.
  if (securityBuffer_ == 0 &&
      usedMemory < availableMemory - 2 * SECURITY_MEMORY_SIZE)
    {
      securityBuffer_ = malloc( 2 * SECURITY_MEMORY_SIZE );
      if (securityBuffer_ != 0) {
	free(securityBuffer_);
	securityBuffer_ = malloc( SECURITY_MEMORY_SIZE );
	if (securityBuffer_) {
	  memoryOverflow = false;
	  deIsolate();
	}
      }
    }

  bool signalMemoryOverflow = false;
  if (memoryOverflow)
    if (securityBuffer_) { // free space to ensure the warning messages will
			   // be sent without problem.
      free (securityBuffer_);
      securityBuffer_ = 0;
      signalMemoryOverflow = true;
    }


  // Scan currently opened connections for ongoing work
  for (std::list<UConnection*>::iterator retr = connectionList.begin();
       retr != connectionList.end();
       retr++)
    if ((*retr)->isActive())  {

      if (!(*retr)->isBlocked())
	(*retr)->continueSend();

      if (signalMemoryOverflow) (*retr)->errorSignal(UERROR_MEMORY_OVERFLOW);
      if (signalcpuoverload) {
	(*retr)->errorSignal(UERROR_CPU_OVERLOAD);
	signalcpuoverload = false;
      }

      (*retr)->errorCheck( UERROR_MEMORY_OVERFLOW );
      (*retr)->errorCheck( UERROR_MEMORY_WARNING );
      (*retr)->errorCheck( UERROR_SEND_BUFFER_FULL );
      (*retr)->errorCheck( UERROR_RECEIVE_BUFFER_FULL );
      (*retr)->errorCheck( UERROR_RECEIVE_BUFFER_CORRUPTED );
      (*retr)->errorCheck( UERROR_CPU_OVERLOAD );

      // Run the connection's command queue:

      if ((*retr)->activeCommand!=0) {

	(*retr)->obstructed = true; // will be changed to 'false' if the whole tree is visited
	(*retr)->treeLock.lock();
	(*retr)->inwork = true;     // to distinguish this call of execute from the one in receive
	(*retr)->execute((*retr)->activeCommand);
	(*retr)->inwork = false;
	(*retr)->treeLock.unlock();
      }

      if ((*retr)->newDataAdded) { // used by loadFile and exec to
				   // delay the parsing after the completion
				   // of execute().

	(*retr)->newDataAdded = false;
	(*retr)->received("");
      }
    }

  // Scan currently opened connections for deleting marked commands or killall order
  if ((reseting) && (stage==0))
    stopall = true;

  for (std::list<UConnection*>::iterator retr = connectionList.begin();
       retr != connectionList.end();
       retr++)
    if  ((*retr)->isActive() && (*retr)->activeCommand)
      {
	if ((*retr)->killall || stopall)
	  {
	    (*retr)->killall = false;
	    delete (*retr)->activeCommand;
	    (*retr)->activeCommand = 0;
	  }
	else if ((*retr)->activeCommand->toDelete)
	  {
	    delete (*retr)->activeCommand;
	    (*retr)->activeCommand = 0;
	  }
	else if (somethingToDelete)
	  (*retr)->activeCommand->deleteMarked();
      }

  somethingToDelete = false;
  stopall = false;

  // Values final assignment and nbAverage reset to 0

  UVarSet selfError;

  for (std::list<UVariable*>::iterator iter = reinitList.begin();
       iter != reinitList.end();)

    if ((*iter)->activity == 2) {
      (*iter)->activity = 0;

      // set previous for stationnary values
      (*iter)->previous3 = (*iter)->previous;
      (*iter)->previous2 = (*iter)->previous;

      iter = reinitList.erase(iter);
    }
    else {
      (*iter)->activity = 2;

      (*iter)->nbAverage = 0;
      (*iter)->reloop = false;

      if (((*iter)->blendType == UMIX) || ((*iter)->blendType == UADD))
	if ((*iter)->value->dataType == DATA_NUM)
	  {
	    if ((*iter)->autoUpdate)
	      selfError = (*iter)->selfSet ( &((*iter)->value->val) );
	    else
	      selfError = (*iter)->selfSet ( &((*iter)->target) );
	  }

      // set previous for next iteration
      (*iter)->previous3 = (*iter)->previous2;
      (*iter)->previous2 = (*iter)->previous;
      (*iter)->previous  = (*iter)->target;

      if ((*iter)->speedmodified)
	(*iter)->reloop = true;
      (*iter)->speedmodified = false;

      iter++;
    }


  // Execute Hub Updaters
  for (urbi::UTimerTable::iterator ittt = urbi::updatemap.begin();
       ittt != urbi::updatemap.end();
       ittt++)
    if ((*ittt)->lastTimeCalled - currentTime + (*ittt)->period <
	frequency_ / 2)
      {
	(*ittt)->call();
	(*ittt)->lastTimeCalled = currentTime;
      }


  //reinitList.clear();
  afterWork();

  updateTime();
  latestTime = lastTime();

  cpuload = (latestTime - currentTime)/getFrequency();

  if (!cpuoverload)
    if  (cpuload > cputhreshold)
      {
	cpucount++;
	if (cpucount > 10) {
	  cpucount = 0;
	  cpuoverload = true;
	  signalcpuoverload = true;
	}
      }
    else if (cpucount > 0)
      cpucount--;

  if (cpuoverload && cpuload < 1)
    {
      cpuoverload = false;
      cpucount = 0;
    }

  // Reseting procedure
  if (reseting) {
    stage++;
    if (stage == 1)
      {
	//delete objects first
	for ( HMvariabletab::iterator retr = variabletab.begin();
	      retr != variabletab.end();
	      retr++)
	  if (((*retr).second->value) &&
	      ((*retr).second->value->dataType == DATA_OBJ))
	    varToReset.push_back( (*retr).second );

	while (!varToReset.empty())
	  {
	    for (std::list<UVariable*>::iterator it = varToReset.begin();
		 it != varToReset.end();)
	      {
		if ((*it)->isDeletable())
		  {
		    delete (*it);
		    it = varToReset.erase(it);
		  }
		else it++;
	      }
	  }

	//delete the rest
	for ( HMvariabletab::iterator retr = variabletab.begin();
	      retr != variabletab.end();
	      retr++)
	  if ((*retr).second->uservar)
	    varToReset.push_back( (*retr).second );

	for (std::list<UVariable*>::iterator it = varToReset.begin();
	     it != varToReset.end();++it)
	  delete *it;

	blocktab.clear();
	freezetab.clear();
	eventtab.clear();

	//variabletab.clear();
	functiontab.clear();  //This leaks awfuly...
	eventtab.clear();
	aliastab.clear();
	objaliastab.clear();
	grouptab.clear();

	varToReset.clear();
	for (std::list<UConnection*>::iterator retr = connectionList.begin();
	     retr != connectionList.end();
	     retr++)
	  if  ((*retr)->isActive())
	    (*retr)->send("*** Reset completed.\n","reset");

	//restart everything
	for (urbi::UStartlist::iterator retr = urbi::objectlist->begin();
	     retr != urbi::objectlist->end();
	     retr++)
	  (*retr)->init((*retr)->name);

	loadFile("URBI.INI",ghost->recvQueue());
	char resetsignal[255];
	strcpy(resetsignal,"var __system__.resetsignal;");
	ghost->recvQueue()->push((const ubyte*)resetsignal,strlen(resetsignal));
	ghost->newDataAdded = true;
      }
    else
      {
	//ASSERT(ghost)
	//ASSERT(ghost->recvQueue())
	//debug("Recv Queue: %d = %s\n",ghost->recvQueue()->dataSize());
	/* char* cc = (char*)ghost->recvQueue()->virtualPop(ghost->recvQueue()->dataSize()-1);
	   for (int i=0;i<ghost->recvQueue()->dataSize()-1;i++)
	   debug("Char%i = %c\n",i,cc[i]);*/
	//}
	HMvariabletab::iterator findResetSignal
	  = variabletab.find("__system__.resetsignal");
	if (findResetSignal != variabletab.end())
	  //if (ghost->recvQueue()->dataSize() == 0)
	  {
	    for (std::list<UConnection*>::iterator retr = connectionList.begin();
		 retr != connectionList.end();
		 retr++)
	      if  ((*retr)->isActive() && (*retr) != ghost)
		{
		  (*retr)->send("*** Reloading\n","reset");

		  loadFile("CLIENT.INI",(*retr)->recvQueue());
		  (*retr)->newDataAdded = true;
		}
	    reseting = false;
	    stage = 0;
	  }
      }
  }
}

//! UServer destructor.
UServer::~UServer()
{
  FREEOBJ(UServer); // Not useful here, but remains for consistency.
}

//! Displays a formatted error message.
/*! This function uses the virtual URobot::display() function to make the
    message printing robot-specific.

    It formats the output in a standard URBI way by adding an ERROR key
    between brackets at the end.
*/
void
UServer::error(const char* s,...)
{
  // This local declaration is rather unefficient but is necessary
  // to insure that the server could be made semi-reentrant.

  char internalMessage_[UServer::MAXSIZE_INTERNALMESSAGE];
  char tmpBuffer_      [UServer::MAXSIZE_INTERNALMESSAGE];

  va_list arg;

  va_start(arg,s);
  vsnprintf(tmpBuffer_,MAXSIZE_INTERNALMESSAGE,s,arg);
  va_end(arg);
  snprintf(internalMessage_,MAXSIZE_INTERNALMESSAGE,
	   "%-90s [ERROR]\n",
	   tmpBuffer_);

  display(internalMessage_);
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
UServer::echo(const char* s,...)
{
  // This local declaration is rather unefficient but is necessary
  // to insure that the server could be made semi-reentrant.

  char internalMessage_[UServer::MAXSIZE_INTERNALMESSAGE];
  char tmpBuffer_      [UServer::MAXSIZE_INTERNALMESSAGE];

  va_list arg;

  va_start(arg,s);
  vsnprintf(tmpBuffer_,MAXSIZE_INTERNALMESSAGE,s,arg);
  va_end(arg);
  snprintf(internalMessage_,MAXSIZE_INTERNALMESSAGE,
	   "%-90s [     ]\n",
	   tmpBuffer_);

  display(internalMessage_);
}

//! Displays a formatted message, with a key
/*! This function uses the virtual URobot::display() function to make the
    message printing robot-specific.
    It formats the output in a standard URBI way by adding a key between
    brackets at the end. This key can be "" or NULL.It can be used to
    visually extract information from the flux of messages printed by
    the server.
    \param key is the message key. Maxlength = 5 chars.
    \param s is the formatted string containing the message.
*/
void
UServer::echoKey(const char* key, const char* s,...)
{
  // This local declaration is rather unefficient but is necessary
  // to insure that the server could be made semi-reentrant.

  char internalMessage_[UServer::MAXSIZE_INTERNALMESSAGE];
  char tmpBuffer_      [UServer::MAXSIZE_INTERNALMESSAGE];
  char key_[6];

  if (key == NULL)
    key_[0] = 0;
  else {
    strncpy(key_,key,5);
    key_[5] = 0;
  }

  va_list arg;

  va_start(arg,s);
  vsnprintf(tmpBuffer_,MAXSIZE_INTERNALMESSAGE,s,arg);
  va_end(arg);
  snprintf(internalMessage_,MAXSIZE_INTERNALMESSAGE,
	   "%-90s [%5s]\n",
	   tmpBuffer_,key_);

  display(internalMessage_);
}

//! Displays a raw message for debug
/*! This function uses the virtual URobot::display() function to make the
    message printing robot-specific.

    \param s is the formatted string containing the message
*/
void
UServer::debug(const char* s,...)
{
  // This local declaration is rather unefficient but is necessary
  // to insure that the server could be made semi-reentrant.

  char tmpBuffer_      [UServer::MAXSIZE_INTERNALMESSAGE];

  va_list arg;

  va_start(arg,s);
  vsnprintf(tmpBuffer_,MAXSIZE_INTERNALMESSAGE,s,arg);
  va_end(arg);

  effectiveDisplay(tmpBuffer_);

//  used to slow down printing with Aibo...
//  ufloat y=4;ufloat x=145;  for (int i=0;i<300000;i++) y = y+ sin( i*x);
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
  return(isolate_);
}

//! Overload this function to specify how your robot is displaying messages.
void
UServer::effectiveDisplay(const char* s)
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
  return (ufloat)0;
}

//! Overload this function to return the remaining power of the robot
/*! The remaining power is expressed as percentage. 0 for empty batteries
    and 1 for full power.
 */
ufloat
UServer::getPower()
{
  return (ufloat)1;
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

    The function should return in header the line corresponding to 'line' or an empty
    string (not NULL!) when there is no line any more.
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
UServer::getCustomHeader (int line, char* header, int maxlength)
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

  if ((usedMemory > (int)(0.8 * availableMemory)) &&
      (warningSent == false)) {

    warningSent = true;

    // Scan currently opened connections
    for (std::list<UConnection*>::iterator retr = connectionList.begin();
	 retr != connectionList.end();
	 retr++)

      if ((*retr)->isActive())
	(*retr)->warning(UWARNING_MEMORY);
  }

  // Hysteresis mechanism
  if ((usedMemory < (int)(0.7 * availableMemory)) &&
      (warningSent == true))
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
  int memo,memo1,memo2;
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
UServer::getVariable ( const char *device,
		       const char *property )
{
  char tmpbuffer[1024];
  HMvariabletab::iterator hmi;

  snprintf(tmpbuffer,1024,"%s.%s",device,property);

  if ((hmi = variabletab.find(tmpbuffer)) != variabletab.end())
    return((*hmi).second);
  else
    return(0);
}



//! Mark all commands with stopTag in all connection for deletion
void
UServer::mark(UString *stopTag)
{
  // Scan currently opened connections for ongoing work
  for (std::list<UConnection*>::iterator retr = connectionList.begin();
       retr != connectionList.end();
       retr++)
    if  ((*retr)->isActive() &&
	 (*retr)->activeCommand)
      (*retr)->activeCommand->mark(stopTag);

  if (parser.commandTree)
    parser.commandTree->mark(stopTag);
}

//! Load a file into the connection.
/*! This function must be redefined by the robot-specific server.
    Returns UFAIL if anything goes wrong, USUCCESS otherwise.
 */
UErrorValue
UServer::loadFile (const char *filename, UCommandQueue* loadQueue)
{
  return USUCCESS;
}

//! Save content to a file
/*! This function must be redefined by the robot-specific server.
    Returns UFAIL if anything goes wrong, USUCCESS otherwise.
 */
UErrorValue
UServer::saveFile (const char *filename, const char * content)
{
  return USUCCESS;
}

//! Add a new connection to the connection list
/*! This function perform also some error testing on the connection
    value and UError return code
*/
void
UServer::addConnection(UConnection *connection)
{
  if (connection == 0 || connection->UError != USUCCESS)
      error(::DISPLAY_FORMAT1,(long)this,
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
       "Connection closed",(long)connection);
  delete connection;
}

//! Adds an alias in the server base
/*! This function is mostly useful for alias creation at start in the
    initialization() function for example.
    return 1 in case of success,  0 if circular alias is detected
*/
int
UServer::addAlias(const char* id, const char* variablename)
{
  if (strcmp(id, variablename)==0)
    return 0;

  const char* newobj = variablename;
  HMaliastab::iterator getobj = ::urbiserver->aliastab.find(newobj);

  while (getobj != ::urbiserver->aliastab.end())
    {
      newobj = (*getobj).second->str();
      if (strcmp(newobj,id)==0)
	return 0;

      getobj = ::urbiserver->aliastab.find(newobj);
    }

  if (aliastab.find(id) != aliastab.end())
    {
      UString *alias = aliastab[id];
      alias->update(variablename);
    }
  else
    {
      UString *ids = new UString(id); // persistant, no delete associated
      aliastab[ids->str()] = new UString(variablename);
    }
  return 1;
}
