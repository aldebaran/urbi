/*! \file userver.hh
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

#ifndef USERVER_HH
# define USERVER_HH

# include <cstdarg>
# include "libport/lockable.hh"
# include "fwd.hh"
# include "utypes.hh"

extern  const char* EXTERNAL_MESSAGE_TAG;
extern  const char* DISPLAY_FORMAT;
extern  const char* DISPLAY_FORMAT1;
extern  const char* DISPLAY_FORMAT2;

extern  const char* MAINDEVICE;

extern  const char* UNKNOWN_TAG;
extern  class UServer   *urbiserver; // Global variable for the server


//! UServer class: handles all URBI system processing.
/*! There must be one UServer defined in the program and it must be overloaded
    to make it specific to the particular robot.

    UServer is used to store the UConnection list and the UDevice list.
    This object does all the internal processing of URBI and handles the pool
    of UCommand's.
*/
class UServer: public libport::Lockable
{
public:
  UServer(ufloat frequency, int freeMemory, const char* mainName);

  virtual ~UServer();

  void              initialization();
  void              work();
  void              main (int argc, const char* argv[]);

  void              error           (const char* s, ...);
  void              echo            (const char* s, ...);
  void              echoKey         (const char* key, const char* s, ...);

  void vdebug (const char* s, va_list args);
  void debug (const char* s, ...);

  void              isolate         ();
  void              deIsolate       ();
  bool              isIsolated      ();

  virtual ufloat    getTime         () = 0;
  virtual ufloat    getPower        () = 0;
  virtual void      getCustomHeader (int line, char* header,
				     int maxlength) = 0;

  /// A list of directory names.
  typedef std::list<std::string> path_type;
  /// Where to look for files to load.
  // Should eventually become an Urbi variable.
  // Should probably be changeable with an envvar.
  // By default, empty (not even ".").  We rely on find_file to
  // return at least the file name.
  path_type path;

  /// Return the full file name, handle paths.
  /// Return \a f on failure.
  virtual std::string find_file (const char* f);

  /// Load a file into the connection.
  /// Returns UFAIL if anything goes wrong, USUCCESS otherwise.
  virtual UErrorValue loadFile      (const char *filename,
				     UCommandQueue* loadQueue);

  /// Save content to a file
  /// This function must be redefined by the robot-specific server.
  /// Returns UFAIL if anything goes wrong, USUCCESS otherwise.
  virtual UErrorValue saveFile      (const char *filename,
				     const char * content) = 0;

  void              memoryCheck     ();
  int               memory          ();

  UVariable*        getVariable     ( const char *device,
				      const char *property);


  ufloat            getFrequency    ();
  void              mark            (UString *stopTag);
  virtual void      reboot          () = 0;
  virtual void      shutdown        () = 0;
  virtual void      beforeWork      ();
  virtual void      afterWork       ();

  /// Display a message on the robot console
  void              display         (const char*);
  /// Display a set of messages on the robot console.
  void              display         (const char**);

  ufloat            lastTime        ();
  void              updateTime      ();
  void              addConnection   (UConnection* connection);
  void              removeConnection(UConnection* connection);
  int               addAlias        (const char* id, const char* variablename);
  UGhostConnection* getGhostConnection ()
  {
    return ghost;
  }

  void              freeze          (const std::string &tag);
  void              unfreeze        (const std::string &tag);
  void              block           (const std::string &tag);
  void              unblock         (const std::string &tag);
  /// List of active connections: includes one UGhostConnection.
  std::list<UConnection*>  connectionList;

  /// Hash of variable values.
  HMvariabletab            variabletab;

  /// Hash of variable that have both an access and change notify
  std::list<UVariable*>    access_and_change_varlist;

  /// Hash of function definition.
  HMfunctiontab            functiontab;
  /// Hash of functions definition markers.
  HMfunctiontab            functiondeftab;
  /// Hash of objects hierarchy.
  HMobjtab                 objtab;
  /// Hash of alias definitions.
  HMaliastab               aliastab;
  /// Hash of obj alias definitions.
  HMaliastab               objaliastab;
  /// Hash of group definitions.
  HMgrouptab               grouptab;
  /// Hash of events.
  HMemittab                emittab;
  /// Hash of function binders.
  HMbindertab              functionbindertab;
  /// Hash of event binders.
  HMbindertab              eventbindertab;
  /// Hash of obj name waiting for a remote new.
  HMobjWaiting             objWaittab;
  /// Hash of all tags currently 'instanciated'
  HMtagtab                 tagtab;
  /// Array of list of UObjects registered for a system messages
  /// The system message type is the index.
  std::vector<std::list<urbi::USystem*> > systemObjects;

  /// Variables to reinit (nbAverage=0).
  std::list<UVariable*>         reinitList;
  /// List of variables to delete after a reset.
  std::list<UVariable*>         resetList;
  /// True when the server is in the process of resting.
  bool                     reseting;
  /// Reseting stage.
  int                      stage;
  /// List of variables to delete in a reset command.
  std::list<UVariable*>         varToReset;

  /// Flag used to signal a memory overflow.
  bool                     memoryOverflow;

  /// Shows debug or not..
  bool                     debugOutput;
  /// Name of the main device.
  UString                  *mainName;
  /// True after a stop command.
  bool                     somethingToDelete;
  /// True after the initialization phase: all vars are uservar then.
  bool                     uservarState;

  /// Cpu load expressed as a number between 0 and 1.
  ufloat                   cpuload;
  /// True when there is a cpu overload.
  bool                     cpuoverload;
  /// A signal must be sent to every connection.
  bool                     signalcpuoverload;
  /// Nb of recent cpu overloads.
  int                      cpucount;
  /// Threshold for cpu overload alert.
  ufloat                   cputhreshold;
  /// True when the server is paranoid on def checking.
  bool                     defcheck;
  ufloat                   previous2Time,
			   previous3Time,
			   currentTime,
			   previousTime,
			   latestTime; ///< used to detect cpu overload
  /// Stops all commands in all connections.
  bool                     stopall;
  /// False inside parsing, true otherwise for commands created by the
  /// kernel.
  bool                     systemcommands;

  /// Urbi TCP Port..
 static const int TCP_PORT            = 54000;

protected:
  virtual void     effectiveDisplay         (const char*) = 0;

private:
  void              mark            (TagInfo*);
  /// Used by echo()& error().
  static const int MAXSIZE_INTERNALMESSAGE = 1024;
  /// Amount of security mem..
  static const int SECURITY_MEMORY_SIZE    = 100000;

  /// Frequency of the calls to work().
  ufloat           frequency_;
  /// Stores memory for emergency use..
  void*            securityBuffer_;
  /// Is the server isolated.
  bool             isolate_;
  /// Store the time on the last call to updateTime();.
  ufloat           lastTime_;
  /// The ghost connection used for URBI.INI.
  UGhostConnection *ghost;
};

//! Accessor for frequency_.
inline ufloat
UServer::getFrequency()
{
  return frequency_;
}

//! Accessor for lastTime_.
inline ufloat
UServer::lastTime()
{
  return lastTime_;
}

extern int URBI_unicID;

inline int unic()
{
  ++URBI_unicID;
  return URBI_unicID;
}

#endif

// Local Variables:
// mode: c++
// End:
