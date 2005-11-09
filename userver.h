/*! \file userver.h
 *******************************************************************************

 File: userver.h\n
 Definition of the USystem class.

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

#ifndef USERVER_H_DEFINED
#define USERVER_H_DEFINED

class UDevice; 
class UConnection;
class UGhostConnection;
class UServer;
class UValue;
class UVariable;

#include <list>
#include <string>
#include <hash_map.h>
#include "parser/uparser.h"
#include "utypes.h"
#include "ufunction.h"
#include "uvariable.h"

class UString;
class UParser;
class UQueue;

using namespace std;

extern  const char* DISPLAY_FORMAT;
extern  const char* DISPLAY_FORMAT1;
extern  const char* DISPLAY_FORMAT2;

extern  const int   NB_HEADER_BEFORE_CUSTOM;
extern  const char* HEADER_BEFORE_CUSTOM[]; 
extern  const int   NB_HEADER_AFTER_CUSTOM;
extern  const char* HEADER_AFTER_CUSTOM[]; 

extern  const char* MAINDEVICE;

extern  const char* UNKNOWN_TAG; 
extern  class UServer   *urbiserver; // Global variable for the server  


//! UServer class: handles all URBI system processing.
/*! There must be one UServer defined in the program and it must be overloaded
    to make it specific to the particular robot.

    UServer is used to store the UConnection list and the UDevice list.
    This object does all the internal processing of URBI and handles the pool
    of UCommand's.

    NOTE ON MULTI THREADING:

    The kernel of the URBI server is not reentrant. There is also a lack for 
    locking mechanisms for access to the command stack and several resources
    shared by all connections. The kernel is designed so far to run in a 
    single threaded application. As long as there is no bi-processor based
    robots, this is safe to assume anyway... 

    As for the flex/bison parser implemented by the UParser class, we have not
    done for the moment any serious test about the pretendous reentrancy of this
    part of the code. However, it is claimed to be reentrant.
    Flex generates a C++ class via the "-+" option, which is 
    supposed to be reentrant because all scanning data is class specific, and 
    bison is invoked with the %pure_parser option which is supposed to make it 
    reentrant. 

    The versions we used to generate the parser are:

    - flex 2.5.4a
    - bison 1.875d
*/
class UServer
{
public:
	
  UServer(double frequency,          
          int freeMemory,
          const char* mainName);

  virtual ~UServer();

  void              initialization();
  void              work();

  void              error           (const char* s,...);
  void              echo            (const char* s,...);
  void              echoKey         (const char* key, const char* s,...);
  void              debug           (const char* s,...);
  void              isolate         ();
  void              deIsolate       ();
  bool              isIsolated      ();

  virtual double    getTime         () = 0;  
  virtual double    getPower        () = 0;  
  virtual void      getCustomHeader (int line, char* header, int maxlength) = 0;
  virtual UErrorValue loadFile      (const char *filename, UCommandQueue* loadQueue) = 0;
  virtual UErrorValue saveFile      (const char *filename, const char * content) = 0;
  virtual void      motor           (bool state) = 0;
  void              memoryCheck     ();                                 
  int               memory          (); 
 
  UVariable*        getVariable     ( const char *device,
                                      const char *property);


  double            getFrequency    ();
  void              mark            (UString *stopTag);
  virtual void      reboot          () = 0;
  virtual void      shutdown        () = 0;
  virtual void      initDevices     () = 0;
  virtual void      beforeWork      ();
  virtual void      afterWork       ();
  
  void              display         (const char*);
  double            lastTime        ();
  void              updateTime      ();
  void              addConnection   (UConnection* connection);
  void              removeConnection(UConnection* connection);
  void              addAlias        (const char* id, const char* variablename);
  void              addCPULoadOffset(double ms);

  list<UConnection*>       connectionList; ///< list of active connections: includes
                                           ///< one UGhostConnection

  HMdevicetab              devicetab; ///< hash of available devices.  
  HMvariabletab            variabletab; ///< hash of variable values  
  HMfunctiontab            functiontab; ///< hash of function definition  
  HMgrouptab               grouptab; ///< hash of group definitions  
  HMaliastab               aliastab; ///< hash of alias definitions
  HMeventtab               eventtab; ///< hash of events

  list<UVariable*>         reinitList; ///< variables to reinit (nbAverage=0)

  hash_map<const char*,
    bool,
    hash<const char*>,
    eqStr>                 blocktab;  ///< hash of the blocked tags.

  hash_map<const char*,
    bool,
    hash<const char*>,
    eqStr>                 freezetab;  ///< hash of the freezed tags.
 
  UParser                  parser; ///< The main parser object
  bool                     memoryOverflow; ///< Flag used to signal a memory 
                                           ///< Overflow.
  bool                     debugOutput;    ///< shows debug or not.
  UString                  *mainName; ///< name of the main device
  bool                     somethingToDelete; ///< true after a stop command
  bool                     uservarState; ///< true after the initialization phase: all vars
                                         ///< are uservar then.
  double                   cpuload; ///< cpu load expressed as a number between 0 and 1
  bool                     cpuoverload; ///< true when there is a cpu overload
  bool                     signalcpuoverload; ///< a signal must be sent to every connection
  int                      cpucount; ///< nb of recent cpu overloads
  double                   cputhreshold; ///< threshold for cpu overload alert
  bool                     defcheck; ///< true when the server is paranoid on def checking
  double                   previous2Time,
                           previous3Time,
                           currentTime, 
                           previousTime, 
                           latestTime; ///< used to detect cpu overload
  double                   cpuloadOffset;
  bool                     stopall; ///< stops all commands in all connections
  bool                     reloadURBIINI; ///< reload URBI.INI file
  bool                     motorstate; ///< state of motor on/off
  bool                     systemcommands; ///< false inside parsing, true otherwise for commands created by the kernel


 static const int TCP_PORT            = 54000; ///< URBI TCP Port.
 
protected:
  
  virtual void     effectiveDisplay         (const char*) = 0;
  
private:
  
  static const int MAXSIZE_INTERNALMESSAGE = 1024;  ///< used by echo()& error()
  static const int SECURITY_MEMORY_SIZE    = 100000;///< amount of security mem.

  double           frequency_; ///< frequency of the calls to work()
  void*            securityBuffer_; ///< stores memory for emergency use.
  bool             isolate_; ///< is the server isolated
  double           lastTime_; ///< store the time on the last call to updateTime();
  UGhostConnection *ghost; ///< the ghost connection used for URBI.INI
};

//! Accessor for frequency_
inline double
UServer::getFrequency() 
{
  return frequency_;
}

//! Accessor for lastTime_
inline double
UServer::lastTime() 
{
  return lastTime_;
}

#endif
