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
# include <sstream>

# include "libport/config.h"
# if ! defined LIBPORT_URBI_ENV_AIBO
#  include <boost/thread.hpp>
# endif

# include "libport/fwd.hh"
# include "libport/compiler.hh"

# include "kernel/fwd.hh"
# include "kernel/ustring.hh"
# include "kernel/utypes.hh"
# include "kernel/tag-info.hh"

//# include "runner/scheduler.hh"
namespace runner
{
  class Scheduler;
}

extern const char* EXTERNAL_MESSAGE_TAG;
extern const char* UNKNOWN_TAG;

extern const char* DISPLAY_FORMAT;
extern const char* DISPLAY_FORMAT1;
extern const char* DISPLAY_FORMAT2;

extern const char* MAINDEVICE;

/// Global variable for the server
extern class UServer* urbiserver;


//! UServer class: handles all URBI system processing.
/*! There must be one UServer defined in the program and it must be overloaded
    to make it specific to the particular robot.

    UServer is used to store the UConnection list and the UDevice list.
    This object does all the internal processing of URBI and handles the pool
    of UCommand's.
*/
class UServer
{
public:
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

   \param mainName FIXME: A comment is missing here

   */
  UServer (ufloat frequency, const char* mainName);

  virtual ~UServer ();

public:
  //! Initialization of the server. Displays the header message & init stuff
  /*! This function must be called once the server is operational and
   able to print messages. It is a requirement for URBI compliance to print
   the header at start, so this function *must* be called. Beside, it also
   do initalization work for the devices and system variables.
   */
  void initialize ();

  //! Main processing loop of the server
  /*! This function must be called every "frequency_" msec to ensure the proper
   functionning of the server. It will call the command execution, the
   connection message sending when they are delayed, etc...

   "frequency_" is a parameter of the server, given in the constructor.
   */
  void work ();

  /// Set the system.args list in URBI.
  void main (int argc, const char* argv[]);


  /// Package information about this server.
  static const libport::PackageInfo& package_info ();

  void error (const char* s, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));
  void echo (const char* s, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));


  //! Display a formatted message, with a key.
  /*! This function uses the virtual URobot::display() function to make the
   message printing robot-specific.
   It formats the output in a standard URBI way by adding a key between
   brackets at the end. This key can be "" or NULL.It can be used to
   visually extract information from the flux of messages printed by
   the server.
   \param key is the message key. Maxlength = 5 chars.
   \param s   is the formatted string containing the message.
   \param args Arguments for the format string.
   */
  void vecho_key (const char* key, const char* s, va_list args)
    __attribute__ ((__format__ (__printf__, 3, 0)));
  void echoKey (const char* key, const char* s, ...)
    __attribute__ ((__format__ (__printf__, 3, 4)));

  /// Send debugging data.
  void vdebug (const char* s, va_list args)
    __attribute__ ((__format__ (__printf__, 2, 0)));
  void debug (const char* s, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));

  void isolate ();
  void deIsolate ();
  bool isIsolated ();

  virtual ufloat getTime () = 0;
  virtual ufloat getPower () = 0;

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
  virtual std::string find_file (const std::string& f);

  /// Load a file into the connection.
  /// Returns UFAIL if anything goes wrong, USUCCESS otherwise.
  virtual UErrorValue loadFile (const std::string& file_name,
				UCommandQueue* loadQueue);

  /// Load \a fn in the ghostqueue.
  UErrorValue load_init_file(const char* fn);

  /// Save content to a file
  /// This function must be redefined by the robot-specific server.
  /// Returns UFAIL if anything goes wrong, USUCCESS otherwise.
  virtual UErrorValue saveFile (const std::string& filename,
				const std::string& content) = 0;

  /// FIXME: Doc?
  UVariable* getVariable (const std::string& device,
			  const std::string& property);

  //! Accessor for frequency_.
  ufloat getFrequency ();
  void mark (UString* stopTag);
  virtual void reboot () = 0;
  virtual void shutdown () = 0;
  //! Function called before work
  /*! Redefine this virtual function if you need to do pre-processing before
    the work function starts.
  */
  virtual void beforeWork ();
  //! Function called after work
  /*! Redefine this virtual function if you need to do post-processing
    before the work function ends.
  */
  virtual void afterWork ();

  /// Display a message on the robot console
  void display (const char*);
  /// Display a set of messages on the robot console.
  void display (const char**);

  //! Accessor for lastTime_.
  ufloat lastTime ();
  void updateTime ();
  void addConnection (UConnection& connection);
  void removeConnection (UConnection& connection);
  int getUID ();
  int addAlias (const std::string& id, const std::string& variablename);

  // A usual connection to stop dependencies.
  UConnection& getGhostConnection ();

  void freeze (const std::string& tag);
  void unfreeze (const std::string& tag);
  void block (const std::string& tag);
  void unblock (const std::string& tag);

  bool isRunningSystemCommands () const;
  void setSystemCommand (bool val);

  /// Hash of all tags currently 'instanciated'
  const HMtagtab& getTagTab () const;
  /// Hash of all tags currently 'instanciated'
  HMtagtab& getTagTab ();

  /// Hash of group definitions.
  const HMgrouptab& getGroupTab () const;
  /// Hash of group definitions.
  HMgrouptab& getGroupTab ();

  /// Hash of group definitions.
  const HMfunctiontab& getFunctionTab () const;
  /// Hash of group definitions.
  HMfunctiontab& getFunctionTab ();

  /// Hash of objects hierarchy.
  const HMobjtab& getObjTab () const;
  /// Hash of objects hierarchy.
  HMobjtab& getObjTab ();

  /// Hash of function binders.
  const HMbindertab& getFunctionBinderTab () const;
  /// Hash of function binders.
  HMbindertab& getFunctionBinderTab ();

  /// Hash of obj alias definitions.
  const HMaliastab& getObjAliasTab () const;
  /// Hash of obj alias definitions.
  HMaliastab& getObjAliasTab ();

  /// True when the server is paranoid on def checking.
  bool isDefChecking () const;
  void setDefCheck (bool val);

  /// Hash of obj name waiting for a remote new.
  const HMobjWaiting& getObjWaitTab () const;
  /// Hash of obj name waiting for a remote new.
  HMobjWaiting& getObjWaitTab ();

  /// Hash of alias definitions.
  const HMaliastab& getAliasTab () const;
  HMaliastab& getAliasTab ();

  /// Hash of variable values.
  const HMvariabletab& getVariableTab () const;
  HMvariabletab& getVariableTab ();

  /// Hash of functions definition markers.
  const HMfunctiontab& getFunctionDefTab () const;
  HMfunctiontab& getFunctionDefTab ();

  /// Hash of event binders.
  const HMbindertab& getEventBinderTab () const;
  HMbindertab& getEventBinderTab ();

  void hasSomethingToDelete ();

  const runner::Scheduler& getScheduler () const;
  runner::Scheduler& getScheduler ();

protected:
  virtual void effectiveDisplay (const char*) = 0;

private:
  friend class TagInfo;

  // Pointer to stop the header dependency.
  runner::Scheduler* scheduler_;

public: // FIXME remove from the public section.
  /// List of active connections: includes one UGhostConnection.
  // FIXME: This is meant to become a runner::Job and move out of this class.
  std::list<UConnection*> connectionList;
private:

  /// FIXME: Comment me.
  void mark (TagInfo*);

  /// \{ Various parts of @c UServer::work.
  /// Execute Timers
  void work_exec_timers_ ();
  /// Access & Change variable list
  void work_access_and_change_ ();
  /// Scan currently opened connections for ongoing work
  void work_handle_connections_ ();
  /// Scan currently opened connections for deleting marked commands or
  /// killall order
  void work_handle_stopall_ ();
  /// Values final assignment and nbAverage reset to 0
  void work_blend_values_ ();
  /// Execute Hub Updaters
  void work_execute_hub_updater_ ();
  void work_test_cpuoverload_ ();
  /// Resetting procedure
  void work_reset_if_needed_ ();
  /// \}

  /// Hash of variable values.
  HMvariabletab variabletab;

public: // FIXME remove from the public section.
  /// Hash of variable that have both an access and change notify
  std::list<UVariable*> access_and_change_varlist;
private:

  /// Hash of function definition.
  HMfunctiontab functiontab;
  /// Hash of functions definition markers.
  HMfunctiontab functiondeftab;
  /// Hash of objects hierarchy.
  HMobjtab objtab;
  /// Hash of alias definitions.
  HMaliastab aliastab;
  /// Hash of obj alias definitions.
  HMaliastab objaliastab;
  /// Hash of group definitions.
  HMgrouptab grouptab;
  /// Hash of events, one entry per (name,nbArgs).
public:
  HMemittab emittab;
  /// Hash of events, one entry per name.
  HMemit2tab emit2tab;
private:
  /// Hash of function binders.
  HMbindertab functionbindertab;
  /// Hash of event binders.
  HMbindertab eventbindertab;
  /// Hash of obj name waiting for a remote new.
  HMobjWaiting objWaittab;
  /// Hash of all tags currently 'instanciated'
  HMtagtab tagtab;

public: // FIXME: remove this from the public section
  /// Array of list of UObjects registered for a system messages
  /// The system message type is the index.
  std::vector<std::list<urbi::USystem*> > systemObjects;

  /// Variables to reinit (nbAverage=0).
  std::list<UVariable*> reinitList;
private:
  /// List of variables to delete after a reset.
  std::list<UVariable*> resetList;
public:
  /// True when the server is in the process of resetting.
  bool resetting;
private:
  /// Resetting stage.
  int stage;
  /// List of variables to delete in a reset command.
  std::list<UVariable*> varToReset;

public:
  /// Shows debug or not.
  bool debugOutput;

# if ! defined LIBPORT_URBI_ENV_AIBO
  /// Used to synchronize message reception.
  boost::recursive_mutex mutex;
# endif

private:
  /// Name of the main device.
  std::string mainName_;

public:
  /// True after a stop command.
  bool somethingToDelete;
  /// True after the initialization phase: all vars are uservar then.
  bool uservarState;

  /// Cpu load expressed as a number between 0 and 1.
  ufloat cpuload;
  /// True when there is a cpu overload.
  bool cpuoverload;
  /// A signal must be sent to every connection.
  bool signalcpuoverload;
  /// Nb of recent cpu overloads.
  int cpucount;
  /// Threshold for cpu overload alert.
  ufloat cputhreshold;
private:
  /// True when the server is paranoid on def checking.
  bool defcheck;

public:
  ufloat previous2Time;
  ufloat previous3Time;
  ufloat currentTime;
  ufloat previousTime;
  ufloat latestTime; ///< used to detect cpu overload

  /// Stops all commands in all connections.
  bool stopall;
private:

  /// False inside parsing, true otherwise for commands created by the
  /// kernel.
  bool systemcommands;

  enum
  {
    /// Urbi TCP Port..
    TCP_PORT = 54000,
    /// Used by echo() & error().
    MAXSIZE_INTERNALMESSAGE = 1024,
  };

  /// Frequency of the calls to work().
  ufloat frequency_;
  /// Is the server isolated.
  bool isolate_;
  /// Store the time on the last call to updateTime();.
  ufloat lastTime_;
  /// The ghost connection used for URBI.INI.
  UGhostConnection* ghost_;

  /// unique id source
  int               uid;
};

/// Unique identifier to create new references.
int unique ();

/// Return an identifier starting with \a prefix, ending with a unique int.
std::string unique (const std::string& prefix);

/*-------------------------.
| Freestanding functions.  |
`-------------------------*/

/// Send debugging messages via ::urbiserver.
void debug (const char* fmt, va_list args)
  __attribute__ ((__format__ (__printf__, 1, 0)));

/// Send debugging messages via ::urbiserver.
void debug (const char* fmt, ...)
  __attribute__ ((__format__ (__printf__, 1, 2)));

/// Send debugging messages indented with \a t spaces, via ::urbiserver.
void debug (unsigned t, const char* fmt, ...)
  __attribute__ ((__format__ (__printf__, 2, 3)));

// Send debugging messages.
# if URBI_DEBUG
// Must be invoked with two pairs of parens.
#  define DEBUG(Msg)  debug Msg
# else
#  define DEBUG(Msg) ((void) 0)
# endif

# include "userver.hxx"

#endif

// Local Variables:
// mode: c++
// End:
